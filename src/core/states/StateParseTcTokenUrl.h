/*!
 * \brief Implements the parsing of TcTokenUrl in header of browser request.
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "context/AuthContext.h"
#include "states/AbstractGenericState.h"

namespace governikus
{

class StateParseTcTokenUrl
	: public AbstractGenericState<AuthContext>
{
	Q_OBJECT
	friend class StateBuilder;

	StateParseTcTokenUrl(const QSharedPointer<WorkflowContext>& pContext);
	virtual void run() override;

};

} /* namespace governikus */
