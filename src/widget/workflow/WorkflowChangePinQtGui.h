/*!
 * \brief Qt widget based WorkflowChangePinUi implementation.
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "context/ChangePinContext.h"
#include "GenericWorkflowGui.h"

namespace governikus
{

class AppQtMainWidget;
class PinSettingsWidget;
class StepErrorGui;


class WorkflowChangePinQtGui
	: public GenericWorkflowGui<ChangePinContext>
{
	Q_OBJECT

	public:
		WorkflowChangePinQtGui(QSharedPointer<ChangePinContext> pContext, AppQtMainWidget* const pParentWidget);
		virtual ~WorkflowChangePinQtGui() override;

		virtual void activate() override;
		virtual void deactivate() override;
		virtual bool verifyAbortWorkflow() override;

	private Q_SLOTS:
		void onStateChanged(const QString& pNextState);

	private:
		PinSettingsWidget* mPinSettingsWidget;
		QSharedPointer<StepErrorGui> mErrorGui;
};

} /* namespace governikus */
