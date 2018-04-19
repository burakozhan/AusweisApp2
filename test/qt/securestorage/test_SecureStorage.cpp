/*!
 * \brief Unit tests for \ref SecureStorage
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#include <QCoreApplication>
#include <QFile>
#include <QtTest>

#include "FileDestination.h"
#include "SecureStorage.h"

#include "asn1/CVCertificate.h"

using namespace governikus;

class test_SecureStorage
	: public QObject
{
	Q_OBJECT
	SecureStorage mSecureStorage;

	private:
		QStringList loadCommentList(const QString& pCommentName)
		{
			const QString& path = FileDestination::getPath("config.json");
			if (!QFile::exists(path))
			{
				qCritical() << "SecureStorage not found";
				return QStringList();
			}

			QFile configFile(path);
			if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				qCritical() << "Wasn't able to open SecureStorage";
				return QStringList();
			}

			QJsonParseError parseError;
			QJsonDocument document = QJsonDocument::fromJson(configFile.readAll(), &parseError);
			configFile.close();
			if (parseError.error != QJsonParseError::NoError)
			{
				qCritical() << "Parse error while reading SecureStorage on position " << parseError.offset << ": " << parseError.errorString();
				return QStringList();
			}

			const QJsonValue commentValues = document.object().value(pCommentName);
			if (commentValues == QJsonValue::Undefined || !commentValues.isArray())
			{
				qCritical() << "Wasn't able to find the array" << pCommentName;
				return QStringList();
			}

			QStringList comments;
			const auto& commentValueArray = commentValues.toArray();
			for (const QJsonValue& comment : commentValueArray)
			{
				if (comment.isString())
				{
					comments += comment.toString();
				}
				else
				{
					qCritical() << "At least one entry of" << pCommentName << "is not a string";
					return QStringList();
				}
			}

			return comments;
		}


	private Q_SLOTS:
		void testGetCVRootCertificatesUnique()
		{
			static const int EXPECTED_CERTIFICATE_COUNT = 10;

			QVector<QSharedPointer<const CVCertificate> > cvcs = CVCertificate::fromHex(mSecureStorage.getCVRootCertificates(true))
					+ CVCertificate::fromHex(mSecureStorage.getCVRootCertificates(false));
			const int count = cvcs.count();
			QCOMPARE(count, EXPECTED_CERTIFICATE_COUNT);

			const QStringList comments = loadCommentList("_comment_2") + loadCommentList("_comment_4");
			QCOMPARE(comments.size(), count);

			// Check that each certificate has a unique car/chr.
			for (int j = 0; j < count; ++j)
			{
				for (int i = 0; i < j; ++i)
				{
					const CVCertificateBody& bodyI = cvcs[i]->getBody();
					const QByteArray carI = bodyI.getCertificationAuthorityReference();
					const QByteArray chrI = bodyI.getCertificateHolderReference();

					const CVCertificateBody& bodyJ = cvcs[j]->getBody();
					const QByteArray carJ = bodyJ.getCertificationAuthorityReference();
					const QByteArray chrJ = bodyJ.getCertificateHolderReference();

					if (carI == carJ && chrI == chrJ)
					{
						qWarning() << "Found 2 certificates with the same name with comments:" << comments[j] << comments[i];
					}

					QVERIFY(carI != carJ || chrI != chrJ);
				}
			}
		}


		void testGetCVRootCertificateNames_data()
		{
			QTest::addColumn<int>("certificateCount");
			QTest::addColumn<bool>("isProductive");
			QTest::addColumn<QString>("commentName");

			QTest::newRow("production") << 3 << true << "_comment_2";
			QTest::newRow("test") << 7 << false << "_comment_4";
		}


		void testGetCVRootCertificateNames()
		{
			QFETCH(int, certificateCount);
			QFETCH(bool, isProductive);
			QFETCH(QString, commentName);

			QVector<QSharedPointer<const CVCertificate> > cvcs = CVCertificate::fromHex(mSecureStorage.getCVRootCertificates(isProductive));
			QCOMPARE(cvcs.count(), certificateCount);

			const QStringList& comments = loadCommentList(commentName);
			QCOMPARE(comments.count(), certificateCount);

			for (int index = 0; index < certificateCount; ++index)
			{
				const QString commentLine = comments[index];

				const QStringList carAndChr = commentLine.split(QStringLiteral("_"));
				QVERIFY(carAndChr.size() == 1 || carAndChr.size() == 2);

				const QByteArray carInComment = carAndChr.first().toUtf8();
				const QByteArray carInCertificate = cvcs.at(index)->getBody().getCertificationAuthorityReference();
				const QByteArray chrInComment = carAndChr.size() == 1 ? carAndChr.first().toUtf8() : carAndChr.last().toUtf8();
				const QByteArray chrInCertificate = cvcs.at(index)->getBody().getCertificateHolderReference();

				if (carInComment != carInCertificate || chrInComment != chrInCertificate)
				{
					qWarning() << "Wrong certificate information at index" << index << "with comment" << commentLine;
				}

				QCOMPARE(carInComment, carInCertificate);
				QCOMPARE(chrInComment, chrInCertificate);
			}
		}


		void testGetUpdateCertificate_data()
		{
			QTest::addColumn<int>("index");
			QTest::addColumn<QString>("subjectInfo");
			QTest::addColumn<QString>("issuerInfo");
			QTest::addColumn<QString>("expiryDate");

			QTest::newRow("production") << 0 << "appl.governikus-asp.de" << "TeleSec ServerPass CA 2" << "2018-05-25T23:59:59Z";
			QTest::newRow("ci") << 1 << "*.tf.bos-test.de" << "govkgrootca" << "2018-12-29T09:59:02Z";
			QTest::newRow("production_2020") << 2 << "appl.governikus-asp.de" << "TeleSec ServerPass Class 2 CA" << "2020-12-06T23:59:59Z";
		}


		void testGetUpdateCertificate()
		{
			const auto& certificates = mSecureStorage.getUpdateCertificates();
			QCOMPARE(certificates.count(), 3);

			QFETCH(int, index);
			QFETCH(QString, subjectInfo);
			QFETCH(QString, issuerInfo);
			QFETCH(QString, expiryDate);

			QVERIFY(certificates.count() - index > 0);

			const auto& cert = certificates.at(index);
			QCOMPARE(cert.subjectInfo(QSslCertificate::CommonName).at(0), subjectInfo);
			QCOMPARE(cert.issuerInfo(QSslCertificate::CommonName).at(0), issuerInfo);
			QCOMPARE(cert.expiryDate(), QDateTime::fromString(expiryDate, Qt::ISODate));
		}


		void testGetSelfAuthentication()
		{
			QVERIFY(mSecureStorage.getSelfAuthenticationUrl(false).isValid());
			QVERIFY(mSecureStorage.getSelfAuthenticationUrl(true).isValid());
		}


		void testGetUpdateServerBaseUrl()
		{
			QVERIFY(mSecureStorage.getUpdateServerBaseUrl().isValid());
		}


		void testAppcast()
		{
			QCOMPARE(mSecureStorage.getAppcastUpdateUrl(), QUrl("https://appl.governikus-asp.de/ausweisapp2/Appcast.json"));
			QCOMPARE(mSecureStorage.getAppcastBetaUpdateUrl(), QUrl("https://appl.governikus-asp.de/ausweisapp2/beta/Appcast.json"));
		}


		void testMinStaticKeySizes()
		{
			QCOMPARE(mSecureStorage.getMinimumStaticKeySize(QSsl::KeyAlgorithm::Rsa), 2000);
			QCOMPARE(mSecureStorage.getMinimumStaticKeySize(QSsl::KeyAlgorithm::Dsa), 2000);
			QCOMPARE(mSecureStorage.getMinimumStaticKeySize(QSsl::KeyAlgorithm::Ec), 224);
		}


		void testMinEphemeralKeySizes()
		{
			QCOMPARE(mSecureStorage.getMinimumEphemeralKeySize(QSsl::KeyAlgorithm::Rsa), 2000);
			QCOMPARE(mSecureStorage.getMinimumEphemeralKeySize(QSsl::KeyAlgorithm::Dsa), 2000);
			QCOMPARE(mSecureStorage.getMinimumEphemeralKeySize(QSsl::KeyAlgorithm::Ec), 224);
		}


		void testSignatureAlgorithms()
		{
			#ifndef GOVERNIKUS_QT
			QSKIP("SignatureAlgorithms not supported");
			#endif

			const auto& tlsSettings = mSecureStorage.getTlsConfig();
			QCOMPARE(tlsSettings.getSignatureAlgorithms().size(), 12);
			QCOMPARE(tlsSettings.getSignatureAlgorithms().constFirst().first, QSsl::KeyAlgorithm::Rsa);
			QCOMPARE(tlsSettings.getSignatureAlgorithms().constFirst().second, QCryptographicHash::Algorithm::Sha512);
			QCOMPARE(tlsSettings.getSignatureAlgorithms().constLast().first, QSsl::KeyAlgorithm::Ec);
			QCOMPARE(tlsSettings.getSignatureAlgorithms().constLast().second, QCryptographicHash::Algorithm::Sha224);
		}


		void testSignatureAlgorithmsPsk()
		{
			#ifndef GOVERNIKUS_QT
			QSKIP("SignatureAlgorithms not supported");
			#endif

			const auto& tlsSettings = mSecureStorage.getTlsConfig(SecureStorage::TlsSuite::PSK);
			QCOMPARE(tlsSettings.getSignatureAlgorithms().size(), 4);
			QCOMPARE(tlsSettings.getSignatureAlgorithms().constFirst().first, QSsl::KeyAlgorithm::Rsa);
			QCOMPARE(tlsSettings.getSignatureAlgorithms().constFirst().second, QCryptographicHash::Algorithm::Sha512);
			QCOMPARE(tlsSettings.getSignatureAlgorithms().constLast().first, QSsl::KeyAlgorithm::Rsa);
			QCOMPARE(tlsSettings.getSignatureAlgorithms().constLast().second, QCryptographicHash::Algorithm::Sha224);
		}


		void testSignatureAlgorithmsRemoteReader()
		{
			#ifndef GOVERNIKUS_QT
			QSKIP("SignatureAlgorithms not supported");
			#endif

			const auto& config = mSecureStorage.getTlsConfigRemote();
			QCOMPARE(config.getSignatureAlgorithms().size(), 3);

			const auto& configPairing = mSecureStorage.getTlsConfigRemote();
			QCOMPARE(configPairing.getSignatureAlgorithms().size(), 3);
		}


		void orderOfCiphers()
		{
			const auto& ciphersForwardSecrecy = mSecureStorage.getTlsConfig().getCiphers();
			QCOMPARE(ciphersForwardSecrecy.first(), QSslCipher("ECDHE-ECDSA-AES256-GCM-SHA384"));
			QCOMPARE(ciphersForwardSecrecy.last(), QSslCipher("DHE-RSA-AES128-SHA"));

			const auto& ciphersPsk = mSecureStorage.getTlsConfig(SecureStorage::TlsSuite::PSK).getCiphers();
			QCOMPARE(ciphersPsk.count(), 5);
			QCOMPARE(ciphersPsk.first(), QSslCipher("RSA-PSK-AES256-GCM-SHA384"));
			QCOMPARE(ciphersPsk.last(), QSslCipher("RSA-PSK-AES256-CBC-SHA"));

			const auto& ciphersEc = mSecureStorage.getTlsConfig().getEllipticCurves();
			QCOMPARE(ciphersEc.count(), 6);
			QCOMPARE(ciphersEc.first(), QSslEllipticCurve::fromLongName("brainpoolP512r1"));
			QCOMPARE(ciphersEc.last(), QSslEllipticCurve::fromLongName("secp224r1"));

			const auto& ciphersEcRemoteReader = mSecureStorage.getTlsConfigRemote().getEllipticCurves();
			QCOMPARE(ciphersEcRemoteReader.count(), 6);
			QCOMPARE(ciphersEcRemoteReader.first(), QSslEllipticCurve::fromLongName("brainpoolP512r1"));
			QCOMPARE(ciphersEcRemoteReader.last(), QSslEllipticCurve::fromLongName("secp224r1"));

			const auto& ciphersEcRemoteReaderPairing = mSecureStorage.getTlsConfigRemote(SecureStorage::TlsSuite::PSK).getEllipticCurves();
			QCOMPARE(ciphersEcRemoteReaderPairing.count(), 0);

			const auto& ciphersRemoteReader = mSecureStorage.getTlsConfigRemote(SecureStorage::TlsSuite::PSK).getCiphers();
			QCOMPARE(ciphersRemoteReader.count(), 5);
			QCOMPARE(ciphersRemoteReader.first(), QSslCipher("RSA-PSK-AES256-GCM-SHA384"));
			QCOMPARE(ciphersRemoteReader.last(), QSslCipher("RSA-PSK-AES256-CBC-SHA"));
		}


		void getSslProtocolVersion()
		{
			QCOMPARE(mSecureStorage.getTlsConfig().getProtocolVersion(), QSsl::SslProtocol::TlsV1_0OrLater);
		}


		void getSslProtocolVersionPsk()
		{
			QCOMPARE(mSecureStorage.getTlsConfig(SecureStorage::TlsSuite::PSK).getProtocolVersion(), QSsl::SslProtocol::TlsV1_1OrLater);
		}


		void getConfiguration_data()
		{
			QTest::addColumn<QSslConfiguration>("configuration");
			QTest::addColumn<int>("cipherSize");

#if OPENSSL_VERSION_NUMBER < 0x10100000L
			QTest::newRow("ciphers non PSK") << mSecureStorage.getTlsConfig().getConfiguration() << 24;
#else
			QTest::newRow("ciphers non PSK") << mSecureStorage.getTlsConfig().getConfiguration() << 18;
#endif

			QTest::newRow("ciphers for PSK") << mSecureStorage.getTlsConfig(SecureStorage::TlsSuite::PSK).getConfiguration() << 5;
			QTest::newRow("remote reader") << mSecureStorage.getTlsConfigRemote().getConfiguration() << 7;
			QTest::newRow("remote reader pairing") << mSecureStorage.getTlsConfigRemote(SecureStorage::TlsSuite::PSK).getConfiguration() << 5;
		}


		void getConfiguration()
		{
			QFETCH(QSslConfiguration, configuration);
			QFETCH(int, cipherSize);

			QCOMPARE(configuration.ciphers().size(), cipherSize);
		}


};

QTEST_GUILESS_MAIN(test_SecureStorage)
#include "test_SecureStorage.moc"
