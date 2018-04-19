/*!
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "generic/ButtonState.h"

#include <QScopedPointer>
#include <QSharedPointer>
#include <QWidget>

namespace governikus
{

class WorkflowContext;


class StepGuiDelegate
	: public QObject
{
	Q_OBJECT

	public:
		StepGuiDelegate();

	Q_SIGNALS:
		void setForwardButtonState(ButtonState pState, const QString& pText);
		void setCancelButtonState(ButtonState pState);
};


class StepGui
	: public QObject
{
	Q_OBJECT

	public:
		StepGui(const QSharedPointer<WorkflowContext>& pContext);
		virtual ~StepGui();

		StepGuiDelegate* getStepGuiDelegate() const
		{
			return mDelegate.data();
		}


		virtual void activate()
		{
		}


		virtual void deactivate()
		{
		}


		virtual void forwardStep();

	protected:
		void setForwardButtonState(ButtonState pState, const QString& pText = QString())
		{
			Q_EMIT mDelegate->setForwardButtonState(pState, pText);
		}


		void setCancelButtonState(ButtonState pState)
		{
			Q_EMIT mDelegate->setCancelButtonState(pState);
		}


	protected:
		QScopedPointer<StepGuiDelegate> mDelegate;

	Q_SIGNALS:
		//void fireErrorMessage(QString pTitle, QString pMessage, QVector<ReaderInfo> pReaderInfos, bool closeErrorMessage = false);
		void fireUiFinished();
		void fireCancelled();

};

} /* namespace governikus */
