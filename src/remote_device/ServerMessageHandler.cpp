/*!
 * \copyright Copyright (c) 2017-2018 Governikus GmbH & Co. KG, Germany
 */

#include "ServerMessageHandler.h"

#include "AppSettings.h"
#include "Env.h"
#include "messages/GetIfdStatus.h"
#include "messages/IfdConnect.h"
#include "messages/IfdConnectResponse.h"
#include "messages/IfdDisconnect.h"
#include "messages/IfdDisconnectResponse.h"
#include "messages/IfdError.h"
#include "messages/IfdEstablishContext.h"
#include "messages/IfdEstablishPaceChannel.h"
#include "messages/IfdEstablishPaceChannelResponse.h"
#include "messages/IfdModifyPin.h"
#include "messages/IfdModifyPinResponse.h"
#include "messages/IfdStatus.h"
#include "messages/IfdTransmit.h"
#include "messages/IfdTransmitResponse.h"
#include "PinModifyOutput.h"
#include "ReaderManager.h"
#include "RemoteDispatcher.h"

#include <QLoggingCategory>


Q_DECLARE_LOGGING_CATEGORY(remote_device)


namespace governikus
{

template<> ServerMessageHandler* createNewObject<ServerMessageHandler*, QSharedPointer<governikus::DataChannel>&>(QSharedPointer<DataChannel>& pChannel)
{
	return new ServerMessageHandlerImpl(pChannel);
}


ServerMessageHandler::~ServerMessageHandler()
{
}


ServerMessageHandlerImpl::ServerMessageHandlerImpl(const QSharedPointer<DataChannel>& pDataChannel)
	: ServerMessageHandler()
	, MessageReceiver()
	, mReaderManager(Env::getSingleton<ReaderManager>())
	, mRemoteDispatcher(Env::create<RemoteDispatcher*>(pDataChannel), &QObject::deleteLater)
	, mCardConnections()
{
	connect(mRemoteDispatcher.data(), &RemoteDispatcher::fireReceived, this, &ServerMessageHandlerImpl::onReceived);
	connect(mRemoteDispatcher.data(), &RemoteDispatcher::fireClosed, this, &ServerMessageHandlerImpl::onClosed);

	connect(&ReaderManager::getInstance(), &ReaderManager::fireReaderAdded, this, &ServerMessageHandlerImpl::onReaderChanged);
	connect(&ReaderManager::getInstance(), &ReaderManager::fireReaderRemoved, this, &ServerMessageHandlerImpl::onReaderRemoved);
	connect(&ReaderManager::getInstance(), &ReaderManager::fireReaderPropertiesUpdated, this, &ServerMessageHandlerImpl::onReaderChanged);
	connect(&ReaderManager::getInstance(), &ReaderManager::fireCardInserted, this, &ServerMessageHandlerImpl::onReaderChanged);
	connect(&ReaderManager::getInstance(), &ReaderManager::fireCardRemoved, this, &ServerMessageHandlerImpl::onReaderChanged);
	connect(&ReaderManager::getInstance(), &ReaderManager::fireCardRetryCounterChanged, this, &ServerMessageHandlerImpl::onReaderChanged);
}


void ServerMessageHandlerImpl::process(const QSharedPointer<const GetIfdStatus>& pMessage)
{
	if (!pMessage->getSlotName().isEmpty())
	{
		const auto& readerInfo = mReaderManager->getReaderInfo(pMessage->getSlotName());
		const QSharedPointer<IfdStatus> ifdStatusMsg(new IfdStatus(readerInfo));
		mRemoteDispatcher->send(ifdStatusMsg);

		return;
	}

	const auto& readerInfos = mReaderManager->getReaderInfos();
	for (const auto& readerInfo : readerInfos)
	{
		if (!readerInfo.isBasicReader())
		{
			continue;
		}

		const QSharedPointer<IfdStatus> ifdStatusMsg(new IfdStatus(readerInfo));
		mRemoteDispatcher->send(ifdStatusMsg);
	}
}


void ServerMessageHandlerImpl::process(const QSharedPointer<const IfdConnect>& pMessage)
{
	const auto& info = mReaderManager->getReaderInfo(pMessage->getSlotName());
	if (!info.isConnected())
	{
		qCWarning(remote_device) << "Unknown reader" << pMessage->getSlotName();
		const QSharedPointer<IfdConnectResponse> response(new IfdConnectResponse(pMessage->getSlotName(), QStringLiteral("/ifdl/terminal#unknownSlot")));
		mRemoteDispatcher->send(response);
		return;
	}

	if (!info.hasEidCard())
	{
		qCWarning(remote_device) << "Cannot determine eID card for reader" << pMessage->getSlotName();
		const QSharedPointer<IfdConnectResponse> response(new IfdConnectResponse(pMessage->getSlotName(), QStringLiteral("/al/common#unknownError")));
		mRemoteDispatcher->send(response);
		return;
	}

	if (mCardConnections.contains(pMessage->getSlotName()))
	{
		qCWarning(remote_device) << "Card is already connected" << pMessage->getSlotName();
		const QSharedPointer<IfdConnectResponse> response(new IfdConnectResponse(pMessage->getSlotName(), QStringLiteral("/al/common#unknownError")));
		mRemoteDispatcher->send(response);
		return;
	}

	qCDebug(remote_device) << "Connect card" << pMessage->getSlotName();
	mReaderManager->callCreateCardConnectionCommand(pMessage->getSlotName(), this, &ServerMessageHandlerImpl::onCreateCardConnectionCommandDone);
}


void ServerMessageHandlerImpl::onCreateCardConnectionCommandDone(QSharedPointer<CreateCardConnectionCommand> pCommand)
{
	qCDebug(remote_device) << "Card connection command completed";
	if (pCommand->getCardConnection() == nullptr)
	{
		qCWarning(remote_device) << "Cannot connect card" << pCommand->getReaderName();
		const QSharedPointer<IfdConnectResponse> response(new IfdConnectResponse(pCommand->getReaderName(), QStringLiteral("/al/common#unknownError")));
		mRemoteDispatcher->send(response);
		return;
	}

	QString slotHandle = QUuid::createUuid().toString();
	qCInfo(remote_device) << "Card successfully connected" << pCommand->getReaderName() << ", using handle " << slotHandle;
	mCardConnections.insert(slotHandle, pCommand->getCardConnection());

	const QSharedPointer<IfdConnectResponse> response(new IfdConnectResponse(slotHandle));
	mRemoteDispatcher->send(response);
}


QString ServerMessageHandlerImpl::convertSlotHandleBackwardsCompatibility(const QString& pSlotHandle)
{
	if (!mCardConnections.contains(pSlotHandle))
	{
		const auto& slotHandles = mCardConnections.keys();
		for (const auto& slotHandle : slotHandles)
		{
			if (mCardConnections[slotHandle]->getReaderInfo().getName() == pSlotHandle)
			{
				return slotHandle;
			}
		}
	}
	return pSlotHandle;
}


void ServerMessageHandlerImpl::process(const QSharedPointer<const IfdDisconnect>& pMessage)
{
	QString slotHandle = pMessage->getSlotHandle();
	slotHandle = convertSlotHandleBackwardsCompatibility(slotHandle);

	if (!mCardConnections.contains(slotHandle))
	{
		qCWarning(remote_device) << "Card is not connected" << slotHandle;
		const QSharedPointer<IfdDisconnectResponse> response(new IfdDisconnectResponse(slotHandle, QStringLiteral("/ifdl/common#invalidSlotHandle")));
		mRemoteDispatcher->send(response);
		return;
	}

	mCardConnections.remove(slotHandle);
	qCInfo(remote_device) << "Card successfully disconnected" << slotHandle;
	const QSharedPointer<IfdDisconnectResponse> response(new IfdDisconnectResponse(slotHandle));
	mRemoteDispatcher->send(response);
}


void ServerMessageHandlerImpl::process(const QSharedPointer<const IfdTransmit>& pMessage)
{
	QString slotHandle = pMessage->getSlotHandle();
	slotHandle = convertSlotHandleBackwardsCompatibility(slotHandle);

	if (!mCardConnections.contains(slotHandle))
	{
		qCWarning(remote_device) << "Card is not connected" << slotHandle;
		const QSharedPointer<IfdTransmitResponse> response(new IfdTransmitResponse(slotHandle, QByteArray(), QStringLiteral("/ifdl/common#invalidSlotHandle")));
		mRemoteDispatcher->send(response);
		return;
	}

	const QSharedPointer<CardConnection>& cardConnection = mCardConnections.value(slotHandle);
	const auto& commandApdu = pMessage->getInputApdu();

	const bool pinPadMode = Env::getSingleton<AppSettings>()->getRemoteServiceSettings().getPinPadMode();
	if (pinPadMode && CommandApdu::isSecureMessaging(commandApdu))
	{
		const bool stopped = cardConnection->stopSecureMessaging();
		if (stopped)
		{
			qCDebug(remote_device) << "The eService has established Secure Messaging. Stopping local Secure Messaging.";
		}
	}

	qCDebug(remote_device) << "Transmit card APDU for" << slotHandle;
	InputAPDUInfo inputApduInfo(commandApdu, MSEBuilder::isUpdateRetryCounterCommand(commandApdu));
	cardConnection->callTransmitCommand(this, &ServerMessageHandlerImpl::onTransmitCardCommandDone, {inputApduInfo}, slotHandle);
}


void ServerMessageHandlerImpl::process(const QSharedPointer<const IfdEstablishPaceChannel>& pMessage)
{
	QString slotHandle = pMessage->getSlotHandle();
	slotHandle = convertSlotHandleBackwardsCompatibility(slotHandle);

	const bool pinPadMode = Env::getSingleton<AppSettings>()->getRemoteServiceSettings().getPinPadMode();
	if (!pinPadMode)
	{
		qCWarning(remote_device) << "EstablishPaceChannel is only available in pin pad mode.";
		const QSharedPointer<IfdEstablishPaceChannelResponse> response(new IfdEstablishPaceChannelResponse(slotHandle, QByteArray(), QStringLiteral("/al/common#unknownError")));
		mRemoteDispatcher->send(response);
		return;
	}

	if (!mCardConnections.contains(slotHandle))
	{
		qCWarning(remote_device) << "Card is not connected" << slotHandle;
		const QSharedPointer<IfdEstablishPaceChannelResponse> response(new IfdEstablishPaceChannelResponse(slotHandle, QByteArray(), QStringLiteral("/ifdl/common#invalidSlotHandle")));
		mRemoteDispatcher->send(response);
		return;
	}

	QSharedPointer<CardConnection> connection = mCardConnections[slotHandle];
	Q_EMIT fireEstablishPaceChannel(pMessage, connection);
}


void ServerMessageHandlerImpl::sendEstablishPaceChannelResponse(const QString& pSlotHandle, const EstablishPACEChannelOutput& pChannelOutput)
{
	const QByteArray& ccid = pChannelOutput.toCcid();
	if (pChannelOutput.getPaceReturnCode() == CardReturnCode::UNKNOWN)
	{
		const QSharedPointer<IfdEstablishPaceChannelResponse> response(new IfdEstablishPaceChannelResponse(pSlotHandle, ccid, QStringLiteral("/al/common#unknownError")));
		mRemoteDispatcher->send(response);
		return;
	}

	const QSharedPointer<IfdEstablishPaceChannelResponse> response(new IfdEstablishPaceChannelResponse(pSlotHandle, ccid));
	mRemoteDispatcher->send(response);
}


void ServerMessageHandlerImpl::process(const QSharedPointer<const IfdModifyPin>& pMessage)
{
	QString slotHandle = pMessage->getSlotHandle();
	slotHandle = convertSlotHandleBackwardsCompatibility(slotHandle);

	const bool pinPadMode = Env::getSingleton<AppSettings>()->getRemoteServiceSettings().getPinPadMode();
	if (!pinPadMode)
	{
		qCWarning(remote_device) << "ModifyPin is only available in pin pad mode.";
		const QSharedPointer<IfdModifyPinResponse> response(new IfdModifyPinResponse(slotHandle, QByteArray(), QStringLiteral("/al/common#unknownError")));
		mRemoteDispatcher->send(response);
		return;
	}

	if (!mCardConnections.contains(slotHandle))
	{
		qCWarning(remote_device) << "Card is not connected" << slotHandle;
		const QSharedPointer<IfdModifyPinResponse> response(new IfdModifyPinResponse(slotHandle, QByteArray(), QStringLiteral("/ifdl/common#invalidSlotHandle")));
		mRemoteDispatcher->send(response);
		return;
	}

	Q_EMIT fireModifyPin(pMessage, mCardConnections[slotHandle]);
}


void ServerMessageHandlerImpl::sendModifyPinResponse(const QString& pSlotHandle, const ResponseApdu& pResponseApdu)
{
	PinModifyOutput pinModifyOutput(pResponseApdu);
	const QByteArray& ccid = pinModifyOutput.toCcid();

	QString minor;
	switch (pResponseApdu.getReturnCode())
	{
		case StatusCode::SUCCESS:
			break;

		case StatusCode::EMPTY:
			minor = QStringLiteral("/ifdl/terminal#noCard");
			break;

		case StatusCode::INPUT_TIMEOUT:
			minor = QStringLiteral("/ifdl/common#timeoutError");
			break;

		case StatusCode::INPUT_CANCELLED:
			minor = QStringLiteral("/ifdl#cancellationByUser");
			break;

		case StatusCode::PASSWORDS_DIFFER:
			minor = QStringLiteral("/ifdl/IO#repeatedDataMismatch");
			break;

		case StatusCode::PASSWORD_OUTOF_RANGE:
			minor = QStringLiteral("/ifdl/IO#unknownPINFormat");
			break;

		default:
			minor = QStringLiteral("/al/common#unknownError");
	}

	const QSharedPointer<IfdModifyPinResponse> response(new IfdModifyPinResponse(pSlotHandle, ccid, minor));
	mRemoteDispatcher->send(response);
}


void ServerMessageHandlerImpl::onTransmitCardCommandDone(QSharedPointer<BaseCardCommand> pCommand)
{
	auto transmitCommand = pCommand.staticCast<TransmitCommand>();
	QString slotHandle = transmitCommand->getSlotHandle();
	slotHandle = convertSlotHandleBackwardsCompatibility(slotHandle);

	if (transmitCommand->getReturnCode() != CardReturnCode::OK)
	{
		qCWarning(remote_device) << "Card transmit for" << slotHandle << "failed" << transmitCommand->getReturnCode();
		QSharedPointer<IfdTransmitResponse> response(new IfdTransmitResponse(slotHandle, QByteArray(), QStringLiteral("/al/common#unknownError")));
		mRemoteDispatcher->send(response);
		return;
	}

	QByteArray responseApdu;
	Q_ASSERT(transmitCommand->getOutputApduAsHex().size() == 1); // may not happen, see TransmitCommand
	if (transmitCommand->getOutputApduAsHex().size() == 1)
	{
		responseApdu = QByteArray::fromHex(transmitCommand->getOutputApduAsHex().first());
	}
	qCInfo(remote_device) << "Card transmit succeeded" << slotHandle;
	QSharedPointer<IfdTransmitResponse> response(new IfdTransmitResponse(slotHandle, responseApdu));
	mRemoteDispatcher->send(response);
}


void ServerMessageHandlerImpl::unprocessed(const QSharedPointer<const RemoteMessage>& pMessage)
{
	unexpectedMessage(pMessage);
}


void ServerMessageHandlerImpl::unexpectedMessage(const QSharedPointer<const RemoteMessage>& pMessage, bool pSendMessage)
{
	qCWarning(remote_device) << "Received an unexpected message of type:" << pMessage->getType();

	if (pSendMessage)
	{
		const QSharedPointer<const IfdError> errorMessage(new IfdError(QString(), QStringLiteral("/al/common#unknownAPIFunction")));
		QMetaObject::invokeMethod(mRemoteDispatcher.data(), "send", Qt::QueuedConnection, Q_ARG(QSharedPointer<const RemoteMessage>, errorMessage));
	}
}


void ServerMessageHandlerImpl::onClosed()
{
	mCardConnections.clear();

	Q_EMIT fireClosed();
}


void ServerMessageHandlerImpl::onReceived(const QSharedPointer<const RemoteMessage>& pMessage)
{
	const QVector<RemoteCardMessageType> serverMessageTypes({
				RemoteCardMessageType::IFDStatus,
				RemoteCardMessageType::IFDConnectResponse,
				RemoteCardMessageType::IFDDisconnectResponse,
				RemoteCardMessageType::IFDTransmitResponse,
				RemoteCardMessageType::IFDEstablishPACEChannelResponse,
				RemoteCardMessageType::IFDModifyPINResponse
			});

	if (serverMessageTypes.contains(pMessage->getType()))
	{
		unexpectedMessage(pMessage, true);
		return;
	}

	receive(pMessage);
}


void ServerMessageHandlerImpl::onReaderChanged(const QString& pReaderName)
{
	ReaderInfo info = mReaderManager->getReaderInfo(pReaderName);
	if (!info.hasEidCard() && mCardConnections.contains(pReaderName))
	{
		mCardConnections.remove(pReaderName);
		qCInfo(remote_device) << "Removed CardConnection for" << pReaderName;
	}

	mRemoteDispatcher->send(QSharedPointer<IfdStatus>(new IfdStatus(info)));
}


void ServerMessageHandlerImpl::onReaderRemoved(const QString& pReaderName)
{
	mRemoteDispatcher->send(QSharedPointer<IfdStatus>(new IfdStatus(pReaderName)));
}


} /* namespace governikus */
