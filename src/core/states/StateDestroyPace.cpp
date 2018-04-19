/*!
 * \copyright Copyright (c) 2016-2018 Governikus GmbH & Co. KG, Germany
 */


#include "StateDestroyPace.h"


using namespace governikus;

StateDestroyPace::StateDestroyPace(const QSharedPointer<WorkflowContext>& pContext)
	: AbstractGenericState(pContext)
{
}


void StateDestroyPace::run()
{
	auto cardConnection = getContext()->getCardConnection();

	Q_ASSERT(cardConnection);
	mConnections += cardConnection->callDestroyPaceChannelCommand(this, &StateDestroyPace::onDestroyPaceDone);
}


void StateDestroyPace::onDestroyPaceDone(QSharedPointer<BaseCardCommand> )
{
	Q_EMIT fireContinue();
}
