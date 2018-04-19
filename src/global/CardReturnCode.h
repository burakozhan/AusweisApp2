/*!
 * \brief Global error code definitions
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "EnumHelper.h"

namespace governikus
{

class GlobalStatus;


defineEnumType(CardReturnCode,
		UNDEFINED,
		OK,
		CARD_NOT_FOUND,
		UNKNOWN,
		INPUT_TIME_OUT,
		INVALID_CAN,
		INVALID_PIN,
		INVALID_PUK,
		COMMAND_FAILED,
		CANCELLATION_BY_USER,
		NEW_PIN_MISMATCH,
		NEW_PIN_INVALID_LENGTH,
		PIN_BLOCKED,
		PIN_NOT_BLOCKED,
		PUK_INOPERATIVE,
		PROTOCOL_ERROR,
		UNEXPECTED_TRANSMIT_STATUS)


class CardReturnCodeUtil
{
	private:
		CardReturnCodeUtil()
		{

		}


	public:
		static GlobalStatus toGlobalStatus(CardReturnCode pCode);
};

} /* namespace governikus */
