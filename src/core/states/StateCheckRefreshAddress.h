/*!
 * \brief Calls the RefreshAddress of TcToken and checks the certificates.
 * After that it will set RedirectAddress in WorkflowContext.
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "context/AuthContext.h"
#include "states/AbstractGenericState.h"

#include <QNetworkReply>
#include <QScopedPointer>
#include <QSslCertificate>
#include <QSslError>
#include <QSslSocket>

class test_StateCheckRefreshAddress;

namespace governikus
{

class StateCheckRefreshAddress
	: public AbstractGenericState<AuthContext>
{
	Q_OBJECT
	friend class StateBuilder;
	friend class ::test_StateCheckRefreshAddress;

	StateCheckRefreshAddress(const QSharedPointer<WorkflowContext>& pContext);

	private:
		QPointer<QNetworkReply> mReply;
		QUrl mUrl;
		QUrl mSubjectUrl;
		bool mCertificateFetched;
		QVector<QUrl> mVerifiedRefreshUrlHosts;

		bool isMatchingSameOriginPolicyInDevMode() const;
		virtual void run() override;

		QUrl determineSubjectUrl();

		void sendGetRequest();
		void fetchServerCertificate();
		bool checkSslConnectionAndSaveCertificate(const QSslConfiguration& pSslConfiguration);
		void doneSuccess();
		void reportCommunicationError(const GlobalStatus& pStatus);

	private Q_SLOTS:
		void onSslHandshakeDone();
		void onNetworkReply();
		void onSslErrors(const QList<QSslError>& errors);
		void onSslHandshakeDoneFetchingServerCertificate();
		void onNetworkErrorFetchingServerCertificate(QNetworkReply::NetworkError pError);

};

} /* namespace governikus */
