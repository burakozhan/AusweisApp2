/*!
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#include "StepAuthenticationDoneGui.h"

using namespace governikus;


StepAuthenticationDoneGui::StepAuthenticationDoneGui(QSharedPointer<AuthContext> pContext)
	: StepGui(pContext)
{
}


StepAuthenticationDoneGui::~StepAuthenticationDoneGui()
{
}


void StepAuthenticationDoneGui::forwardStep()
{
	Q_EMIT fireCancelled();
}
