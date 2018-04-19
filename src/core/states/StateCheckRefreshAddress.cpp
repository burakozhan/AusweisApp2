/*!
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#include "StateCheckRefreshAddress.h"

#include "AppSettings.h"
#include "CertificateChecker.h"
#include "Env.h"
#include "HttpStatusCode.h"
#include "NetworkManager.h"
#include "StateRedirectBrowser.h"
#include "TlsChecker.h"
#include "UrlUtil.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslCipher>
#include <QSslKey>
#include <QSslSocket>


using namespace governikus;

Q_DECLARE_LOGGING_CATEGORY(developermode)
Q_DECLARE_LOGGING_CATEGORY(network)


StateCheckRefreshAddress::StateCheckRefreshAddress(const QSharedPointer<WorkflowContext>& pContext)
	: AbstractGenericState(pContext, false)
	, mReply(nullptr)
	, mUrl()
	, mSubjectUrl()
	, mCertificateFetched(false)
{
}


bool StateCheckRefreshAddress::isMatchingSameOriginPolicyInDevMode() const
{
	// Checking for same origin policy needs a special treatment in developer mode because
	// a tcTokenURL with the http scheme is acceptable.
	if (!AppSettings::getInstance().getGeneralSettings().isDeveloperMode())
	{
		return false;
	}

	if (mUrl.scheme() == QLatin1String("http") || mSubjectUrl.scheme() == QLatin1String("http"))
	{
		QUrl devModeUrl = mUrl;
		QUrl devModeSubjectUrl = mSubjectUrl;

		// Accept http and ignore redirects, i.e. only compare domains (ignore schemes and ports).
		devModeUrl.setScheme(QStringLiteral("http"));
		devModeSubjectUrl.setScheme(QStringLiteral("http"));
		devModeUrl.setPort(80);
		devModeSubjectUrl.setPort(80);
		const bool matching = UrlUtil::isMatchingSameOriginPolicy(devModeUrl, devModeSubjectUrl);
		if (matching)
		{
			qCCritical(developermode) << "SOP-Check: Ignoring scheme and port in developer mode.";
			qCCritical(developermode).noquote() << "  Origin URL:" << mSubjectUrl.toString();
			qCCritical(developermode).noquote() << "  Refresh URL:" << mUrl.toString();
		}
		return matching;
	}

	return false;
}


void StateCheckRefreshAddress::run()
{
	const auto tcToken = getContext()->getTcToken();

	if (!tcToken)
	{
		qDebug() << "Invalid TCToken";
		Q_EMIT fireContinue();
		return;
	}

	if (tcToken->usePsk())
	{
		// When the tcToken provided a psk, it is necessary to clear
		// all connections to force new connections without psk settings
		// after we finished the authentication communication.
		Env::getSingleton<NetworkManager>()->clearConnections();
	}

	mUrl = tcToken->getRefreshAddress();
	auto refreshAddrError = QStringLiteral("Invalid RefreshAddress: %1").arg(mUrl.toString());
	if (!mUrl.isValid())
	{
		qDebug() << refreshAddrError;
		Q_EMIT fireContinue();
		return;
	}
	if (mUrl.scheme() != QLatin1String("https"))
	{
		if (AppSettings::getInstance().getGeneralSettings().isDeveloperMode())
		{
			qCDebug(developermode) << refreshAddrError;
		}
		else
		{
			qDebug() << refreshAddrError;
			Q_EMIT fireContinue();
			return;
		}
	}

	qDebug() << "Current URL: " << mUrl.toString();
	mSubjectUrl = determineSubjectUrl();
	qDebug() << "SubjectUrl: " << mSubjectUrl.toString();

	if (UrlUtil::isMatchingSameOriginPolicy(mUrl, mSubjectUrl) || isMatchingSameOriginPolicyInDevMode())
	{
		qDebug() << "SOP-Check succeeded, abort process";
		fetchServerCertificate();
	}
	else
	{
		qDebug() << "SOP-Check failed, start process";
		sendGetRequest();
	}
}


QUrl StateCheckRefreshAddress::determineSubjectUrl()
{
	QUrl subjectUrl;
	auto eac1 = getContext()->getDidAuthenticateEac1();
	if (eac1 && eac1->getCertificateDescription() != nullptr)
	{
		subjectUrl = QUrl(eac1->getCertificateDescription()->getSubjectUrl(), QUrl::StrictMode);
		if (!subjectUrl.isValid())
		{
			subjectUrl.clear();
		}
	}

	if (AppSettings::getInstance().getGeneralSettings().isDeveloperMode())
	{
		// Perform SOP-Check against TcToken-URL instead of subjectURL
		subjectUrl = getContext()->getTcTokenUrl();
	}

	if (subjectUrl.isEmpty())
	{
		// according to TR-03124, chapter 2.4.5.1
		qWarning() << "No subjectURL/certificate description available, take the TcToken-URL instead";
		subjectUrl = getContext()->getTcTokenUrl();
	}
	return subjectUrl;
}


void StateCheckRefreshAddress::sendGetRequest()
{
	if (!mReply.isNull())
	{
		mReply->deleteLater();
	}

	qDebug() << "Send GET request to URL: " << mUrl.toString();
	QNetworkRequest request(mUrl);
	mReply = Env::getSingleton<NetworkManager>()->get(request);
	mConnections += connect(mReply.data(), &QNetworkReply::sslErrors, this, &StateCheckRefreshAddress::onSslErrors);
	mConnections += connect(mReply.data(), &QNetworkReply::encrypted, this, &StateCheckRefreshAddress::onSslHandshakeDone);
	mConnections += connect(mReply.data(), &QNetworkReply::finished, this, &StateCheckRefreshAddress::onNetworkReply);
}


void StateCheckRefreshAddress::onSslErrors(const QList<QSslError>& pErrors)
{
	if (TlsChecker::containsFatalError(mReply, pErrors))
	{
		reportCommunicationError(GlobalStatus(GlobalStatus::Code::Network_Ssl_Establishment_Error));
	}
}


void StateCheckRefreshAddress::reportCommunicationError(const GlobalStatus& pStatus)
{
	qCritical() << pStatus;
	updateStatus(pStatus);
	Q_EMIT fireAbort();
}


void StateCheckRefreshAddress::onSslHandshakeDone()
{
	const auto& cfg = mReply->sslConfiguration();
	TlsChecker::logSslConfig(cfg, qInfo(network));

	if (!checkSslConnectionAndSaveCertificate(cfg))
	{
		// checkAndSaveCertificate already set the error
		mReply->abort();
	}
}


bool StateCheckRefreshAddress::checkSslConnectionAndSaveCertificate(const QSslConfiguration& pSslConfiguration)
{
	const QSharedPointer<AuthContext>& context = getContext();
	Q_ASSERT(!context.isNull());

	std::function<void(const QUrl&, const QSslCertificate&)> saveCertificateFunc = [&]
				(const QUrl& pUrl, const QSslCertificate& pCertificate)
			{
				mVerifiedRefreshUrlHosts += pUrl;
				context->addCertificateData(pUrl, pCertificate);
			};

	switch (CertificateChecker::checkAndSaveCertificate(pSslConfiguration.peerCertificate(), mUrl, context->getDidAuthenticateEac1(), context->getDvCvc(), saveCertificateFunc))
	{
		case CertificateChecker::CertificateStatus::Good:
			break;

		case CertificateChecker::CertificateStatus::Unsupported_Algorithm_Or_Length:
			reportCommunicationError(GlobalStatus(GlobalStatus::Code::Workflow_Network_Ssl_Certificate_Unsupported_Algorithm_Or_Length));
			return false;

		case CertificateChecker::CertificateStatus::Hash_Not_In_Description:
			reportCommunicationError(GlobalStatus(GlobalStatus::Code::Workflow_Nerwork_Ssl_Hash_Not_In_Certificate_Description));
			return false;
	}

	if (!TlsChecker::hasValidEphemeralKeyLength(pSslConfiguration.ephemeralServerKey()))
	{
		reportCommunicationError(GlobalStatus::Code::Workflow_Network_Ssl_Connection_Unsupported_Algorithm_Or_Length);
		return false;
	}

	return true;
}


void StateCheckRefreshAddress::onNetworkReply()
{
	int statusCode = mReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	QUrl redirectUrl = mReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	qDebug() << "Status Code: " << statusCode << " | redirect URL: " << redirectUrl;
	for (const auto& header : mReply->rawHeaderPairs())
	{
		qCDebug(network).nospace() << "Header | " << header.first << ": " << header.second;
	}

	if (mReply->error() != QNetworkReply::NoError)
	{
		qCritical() << "An error occured: " << mReply->errorString();
		switch (NetworkManager::toNetworkError(mReply.data()))
		{
			case NetworkManager::NetworkError::ServiceUnavailable:
				reportCommunicationError(GlobalStatus(GlobalStatus::Code::Network_ServiceUnavailable));
				break;

			case NetworkManager::NetworkError::TimeOut:
				reportCommunicationError(GlobalStatus(GlobalStatus::Code::Network_TimeOut));
				break;

			case NetworkManager::NetworkError::ProxyError:
				reportCommunicationError(GlobalStatus(GlobalStatus::Code::Network_Proxy_Error));
				break;

			case NetworkManager::NetworkError::SslError:
				reportCommunicationError(GlobalStatus(GlobalStatus::Code::Network_Ssl_Establishment_Error));
				break;

			case NetworkManager::NetworkError::OtherError:
				reportCommunicationError(GlobalStatus(GlobalStatus::Code::Network_Other_Error));
				break;
		}
		return;
	}

	if (statusCode != HttpStatusCode::FOUND && statusCode != HttpStatusCode::SEE_OTHER && statusCode != HttpStatusCode::TEMPORARY_REDIRECT)
	{
		qCritical() << "Got unexpected status code: " << statusCode;
		reportCommunicationError(GlobalStatus(GlobalStatus::Code::Workflow_Network_Expected_Redirect, QString::number(statusCode)));
		return;
	}

	if (redirectUrl.isEmpty())
	{
		qCritical() << "Got empty redirect URL";
		reportCommunicationError(GlobalStatus(GlobalStatus::Code::Workflow_Network_Empty_Redirect_Url));
		return;
	}

	if (!redirectUrl.isValid())
	{
		qCritical() << "Got malformed redirect URL:" << redirectUrl;
		reportCommunicationError(GlobalStatus(GlobalStatus::Code::Workflow_Network_Malformed_Redirect_Url, QString::fromLatin1(redirectUrl.toEncoded())));
		return;
	}

	if (redirectUrl.scheme() != QLatin1String("https"))
	{
		auto httpsError = QStringLiteral("Redirect URL is not https: %1").arg(redirectUrl.toString());

		if (AppSettings::getInstance().getGeneralSettings().isDeveloperMode())
		{
			qCCritical(developermode) << httpsError;
		}
		else
		{
			qCritical() << httpsError;
			reportCommunicationError(GlobalStatus(GlobalStatus::Code::Workflow_Network_Invalid_Scheme, redirectUrl.scheme()));
			return;
		}
	}

	if (UrlUtil::isMatchingSameOriginPolicy(mUrl, mSubjectUrl) || isMatchingSameOriginPolicyInDevMode())
	{
		qDebug() << "SOP-Check succeeded, abort process";
		mUrl = redirectUrl;
		fetchServerCertificate();
	}
	else
	{
		qDebug() << "SOP-Check failed, continue process";
		mUrl = redirectUrl;
		sendGetRequest();
	}
}


void StateCheckRefreshAddress::fetchServerCertificate()
{
	if (AppSettings::getInstance().getGeneralSettings().isDeveloperMode() && mUrl.scheme() == QLatin1String("http"))
	{
		qCWarning(developermode) << "Refresh URL is http only. Certificate check skipped.";
		doneSuccess();
		return;
	}

	/*
	 * According to Tr-03124-1, page 15 fetching the server certificate is optional if the server certificate was already collected:
	 *
	 * "This state MAY be skipped if a connection to the refresh URL was already established as part of the procedure described above"
	 *
	 * But keep attention: ONLY IF THE SERVER CERTIFICATE WAS COLLECTED DURING DETERMINATION OF THE REFRESH URL *NOT* DURING DETERMINATION OF THE TCTOKEN.
	 */
	if (mVerifiedRefreshUrlHosts.contains(mUrl))
	{
		qDebug() << "SSL certificate already collected for" << mUrl;
		doneSuccess();
		return;
	}

	qDebug() << "Fetch TLS certificate for URL" << mUrl;
	QNetworkRequest request(mUrl);

	// Clear all connections to ensure a fresh connection is established and the
	// encrypted signal is emitted, see Qt documentation QNetworkReply::encrypted():
	//
	// "...This means that you are only guaranteed to receive this signal for the first connection to a site in the lifespan of the QNetworkAccessManager."
	Env::getSingleton<NetworkManager>()->clearConnections();
	mReply = Env::getSingleton<NetworkManager>()->get(request);

	mConnections += connect(mReply.data(), &QNetworkReply::encrypted, this, &StateCheckRefreshAddress::onSslHandshakeDoneFetchingServerCertificate);
	mConnections += connect(mReply.data(), &QNetworkReply::sslErrors, this, &StateCheckRefreshAddress::onSslErrors);
	mConnections += connect(mReply.data(), QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &StateCheckRefreshAddress::onNetworkErrorFetchingServerCertificate);
}


void StateCheckRefreshAddress::onSslHandshakeDoneFetchingServerCertificate()
{
	const auto& cfg = mReply->sslConfiguration();
	TlsChecker::logSslConfig(cfg, qInfo(network));

	if (checkSslConnectionAndSaveCertificate(cfg))
	{
		doneSuccess();
	}
	else
	{
		// checkSslConnectionAndSaveCertificate already set the error
	}
	// just establish the TLS connection but do not perform HTTP request
	mCertificateFetched = true;
	mReply->abort();
}


void StateCheckRefreshAddress::doneSuccess()
{
	getContext()->setRefreshUrl(mUrl);
	qDebug() << "Determined RefreshUrl: " << mUrl;
	Q_EMIT fireContinue();
}


void StateCheckRefreshAddress::onNetworkErrorFetchingServerCertificate(QNetworkReply::NetworkError pError)
{
	if (mCertificateFetched && pError == QNetworkReply::NetworkError::OperationCanceledError)
	{
		return;
	}
	qCritical() << "An error occured fetching the server certificate: " << mReply->errorString();
	reportCommunicationError(GlobalStatus(GlobalStatus::Code::Workflow_Network_Empty_Redirect_Url));
}
