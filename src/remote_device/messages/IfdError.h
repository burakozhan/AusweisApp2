/*!
 * \copyright Copyright (c) 2017-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "RemoteMessageResponse.h"


namespace governikus
{
class IfdError
	: public RemoteMessageResponse
{
	private:
		QString mSlotHandle;

	public:
		IfdError(const QString& pSlotHandle, const QString& pResultMinor = QString());
		IfdError(const QJsonObject& pMessageObject);
		virtual ~IfdError() override = default;

		const QString& getSlotHandle() const;
		virtual QJsonDocument toJson(const QString& pContextHandle) const override;
};


} /* namespace governikus */
