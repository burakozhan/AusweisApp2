/*!
 * \brief Implementation of card object for PC/SC
 *  *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "Card.h"
#include "CardReturnCode.h"
#include "PcscReader.h"
#include "PcscUtils.h"

#include <QByteArray>
#include <QPointer>
#include <QTimer>


namespace governikus
{

class PcscCard
	: public Card
{
	Q_OBJECT

	private:
		QPointer<PcscReader> mReader;
		PCSC_INT mProtocol;
		SCARDCONTEXT mContextHandle;
		SCARDHANDLE mCardHandle;
		QTimer mTimer;

		PCSC_RETURNCODE transmit(const QByteArray& pSendBuffer, QByteArray& pReceiveBuffer);

		PCSC_RETURNCODE transmit(const QByteArray& pSendBuffer,
				QByteArray& pReceiveBuffer,
				const SCARD_IO_REQUEST* pSendPci,
				SCARD_IO_REQUEST& pRecvPci,
				PCSC_INT& pBytesReceived);

		PCSC_RETURNCODE control(PCSC_INT pCntrCode, const QByteArray& pCntrInput, QByteArray& pCntrOutput);

	private Q_SLOTS:
		void sendSCardStatus();

	public:
		PcscCard(PcscReader* pPcscReader);
		virtual ~PcscCard() override;

		virtual CardReturnCode connect() override;
		virtual CardReturnCode disconnect() override;
		virtual bool isConnected() override;

		virtual CardReturnCode transmit(const CommandApdu& pCmd, ResponseApdu& pRes) override;

		virtual CardReturnCode establishPaceChannel(PACE_PASSWORD_ID pPasswordId, const QByteArray& pChat, const QByteArray& pCertificateDescription, EstablishPACEChannelOutput& pChannelOutput, quint8 pTimeoutSeconds) override;

		virtual CardReturnCode destroyPaceChannel() override;

		virtual CardReturnCode setEidPin(quint8 pTimeoutSeconds, ResponseApdu& pResponseApdu) override;
};

} /* namespace governikus */
