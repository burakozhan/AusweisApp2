/*!
 * \brief Unit tests for \ref RemoteServerImpl
 *
 * \copyright Copyright (c) 2017-2018 Governikus GmbH & Co. KG, Germany
 */

#include "Env.h"
#include "RemoteReaderAdvertiser.h"
#include "RemoteServer.h"
#include "RemoteWebSocketServer.h"

#include <QPointer>
#include <QSharedPointer>
#include <QtTest>


using namespace governikus;

class RemoteWebSocketServerMock
	: public RemoteWebSocketServer
{
	Q_OBJECT

	public:
		QString mServerName;
		bool mPairing;
		bool mListening = false, mConnected = false;
		QSharedPointer<ServerMessageHandler> mEmptyHandler;

		bool isListening() const override
		{
			return mListening;
		}


		bool isConnected() const override
		{
			return mConnected;
		}


		void setConnected(bool pConnected)
		{
			if (mConnected != pConnected)
			{
				mConnected = pConnected;
				Q_EMIT fireConnectedChanged(mConnected);
			}
		}


		bool listen(const QString& pServerName) override
		{
			if (mListening)
			{
				return false;
			}
			mServerName = pServerName;
			mListening = true;
			return true;
		}


		void close() override
		{
			mListening = false;
			mServerName = QString();
			mPairing = false;
		}


		QString getServerName() const override
		{
			return mServerName;
		}


		quint16 getServerPort() const override
		{
			return 1;
		}


		void setPairing(bool pEnabled = true) override
		{
			mPairing = pEnabled;
		}


		QSslCertificate getCurrentCertificate() const override
		{
			return QSslCertificate();
		}


		const QSharedPointer<ServerMessageHandler>& getMessageHandler() const override
		{
			return mEmptyHandler;
		}


};


class RemoteReaderAdvertiserMock
	: public RemoteReaderAdvertiser
{
	Q_OBJECT

	public:
		const QString mIfdName;
		const QString mIfdId;
		const quint16 mPort;

		RemoteReaderAdvertiserMock(const QString& pIfdName, const QString& pIfdId, quint16 pPort)
			: RemoteReaderAdvertiser()
			, mIfdName(pIfdName)
			, mIfdId(pIfdId)
			, mPort(pPort)
		{
		}


		virtual ~RemoteReaderAdvertiserMock();
};


RemoteReaderAdvertiserMock::~RemoteReaderAdvertiserMock()
{
}


class test_RemoteServer
	: public QObject
{
	Q_OBJECT

	QPointer<RemoteReaderAdvertiserMock> mAdvertiserMock;
	QPointer<RemoteWebSocketServerMock> mWebSocketMock;
	QScopedPointer<RemoteServer> mServer;

	private Q_SLOTS:
		void init()
		{
			std::function<RemoteReaderAdvertiser*(const QString&, const QString&, quint16&)> creator = [this](const QString& pIfdName, const QString& pIfdId, quint16& pPort){
						mAdvertiserMock = new RemoteReaderAdvertiserMock(pIfdName, pIfdId, pPort);
						return mAdvertiserMock;
					};
			Env::setCreator<RemoteReaderAdvertiser*, const QString&, const QString&, quint16&>(creator);
			std::function<RemoteWebSocketServer*()> creator2 = [this](){
						mWebSocketMock = new RemoteWebSocketServerMock;
						return mWebSocketMock;
					};
			Env::setCreator<RemoteWebSocketServer*>(creator2);
			mServer.reset(new RemoteServerImpl());
		}


		void startStop()
		{
			QVERIFY(!mAdvertiserMock);
			QVERIFY(!mWebSocketMock->isListening());

			QVERIFY(mServer->start(QStringLiteral("ServerName")));
			QVERIFY(mAdvertiserMock);
			QVERIFY(mWebSocketMock->isListening());

			mServer->stop();
			QVERIFY(!mAdvertiserMock);
			QVERIFY(!mWebSocketMock->isListening());
		}


		void serverName()
		{
			mServer->start(QStringLiteral("ServerName"));
			QCOMPARE(mAdvertiserMock->mIfdName, QStringLiteral("ServerName"));
			QCOMPARE(mWebSocketMock->mServerName, QStringLiteral("ServerName"));
		}


		void port()
		{
			mServer->start(QStringLiteral("ServerName"));
			QCOMPARE(mAdvertiserMock->mPort, mWebSocketMock->getServerPort());
		}


		void stopAdvertisingOnConnection()
		{
			mServer->start(QStringLiteral("ServerName"));
			mWebSocketMock->setConnected(true);

			QVERIFY(!mAdvertiserMock);
		}


		void startAdvertisingOnConnectionClosed()
		{
			mServer->start(QStringLiteral("ServerName"));
			mWebSocketMock->setConnected(true);
			mWebSocketMock->setConnected(false);

			QVERIFY(mAdvertiserMock);
		}


};


QTEST_GUILESS_MAIN(test_RemoteServer)
#include "test_RemoteServer.moc"
