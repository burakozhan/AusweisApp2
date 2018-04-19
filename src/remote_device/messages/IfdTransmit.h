/*!
 * \copyright Copyright (c) 2017-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "RemoteMessage.h"

#include <QByteArray>


namespace governikus
{
class IfdTransmit
	: public RemoteMessage
{
	private:
		QString mSlotHandle;
		QByteArray mInputApdu;

		void parseCommandApdu(QJsonValue pEntry);

	public:
		IfdTransmit(const QString& pSlotHandle, const QByteArray& pInputApdu);
		IfdTransmit(const QJsonObject& pMessageObject);
		virtual ~IfdTransmit() override = default;

		const QString& getSlotHandle() const;
		const QByteArray& getInputApdu() const;
		virtual QJsonDocument toJson(const QString& pContextHandle) const override;
};


} /* namespace governikus */
