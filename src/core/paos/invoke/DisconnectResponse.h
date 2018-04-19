/*!
 * \brief Generate information for DisconnectResponse.
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "paos/ResponseType.h"
#include "PaosCreator.h"

class test_DisconnectResponse;

namespace governikus
{

class DisconnectResponse
	: public PaosCreator
	, public ResponseType
{
	friend class ::test_DisconnectResponse;

	private:
		QString mSlotHandle;

		Q_DISABLE_COPY(DisconnectResponse)

		QDomElement createDisconnectResponse();

		virtual QDomElement getDocumentStructure() override;

	public:
		DisconnectResponse();
		void setSlotHandle(const QString& slotHandle);
};

} /* namespace governikus */
