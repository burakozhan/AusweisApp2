/*!
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#include "paos/invoke/InitializeFrameworkResponse.h"
#include "states/StateInitializeFramework.h"

using namespace governikus;

StateInitializeFramework::StateInitializeFramework(const QSharedPointer<WorkflowContext>& pContext)
	: AbstractGenericState(pContext)
{
}


void StateInitializeFramework::run()
{
	Q_ASSERT(!getContext()->getInitializeFramework().isNull());
	Q_EMIT fireContinue();
}
