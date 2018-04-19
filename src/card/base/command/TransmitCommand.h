/*!
 * \brief Command to transmit data to/from the card.
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "BaseCardCommand.h"
#include "InputAPDUInfo.h"

class test_TransmitCommand;

namespace governikus
{

class CardConnection;

class TransmitCommand
	: public BaseCardCommand
{
	Q_OBJECT

	private:
		friend class ::test_TransmitCommand;

		const QVector<InputAPDUInfo> mInputApduInfos;
		const QString mSlotHandle;
		QByteArrayList mOutputApduAsHex;

		static bool isAcceptable(const InputAPDUInfo& pInputApduInfo, const ResponseApdu& pResponse);

	protected:
		virtual void internalExecute() override;
		virtual ~TransmitCommand() override;

	public:
		TransmitCommand(QSharedPointer<CardConnectionWorker> pCardConnectionWorker,
				const QVector<InputAPDUInfo>& pInputApduInfos,
				const QString pSlotHandle);

		const QByteArrayList& getOutputApduAsHex() const
		{
			return mOutputApduAsHex;
		}


		const QString& getSlotHandle() const
		{
			return mSlotHandle;
		}


};

} /* namespace governikus */
