/*!
 * \brief GUI to select reader/card.
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "context/AuthContext.h"
#include "DiagnosisGui.h"
#include "ReaderDeviceGui.h"
#include "ReaderManager.h"
#include "step/StepAuthenticationEac1Widget.h"
#include "StepGui.h"

#include <QMessageBox>

class QLabel;

namespace governikus
{

class AuthenticateStepsWidget;

class StepChooseCardGui
	: public StepGui
{
	Q_OBJECT

	private:
		const QSharedPointer<AuthContext> mContext;
		StepAuthenticationEac1Widget* const mWidget;
		QPointer<QMessageBox> mInformationMessageBox;
		QPointer<DiagnosisGui> mDiagnosisGui;
		QPointer<ReaderDeviceGui> mReaderDeviceGui;
		QPushButton* mCancelButton, * mDeviceButton, * mDiagnosisButton;
		bool mSubDialogOpen;

		QString getCurrentReaderImage(const QVector<ReaderInfo>& pReaderInfos);
		static QString formatErrorMessages(const QString& pMessage1, const QString& pMessage2);
		void updateErrorMessage(const QString& pTitle, const QString& pMessage1, const QString& pMessage2 = QString(), bool closeErrorMessage = false);
		const QString connectedRemoteReaderNames() const;

	private Q_SLOTS:
		void onSubDialogFinished();

	public Q_SLOTS:
		void onReaderManagerSignal();

	public:
		StepChooseCardGui(const QSharedPointer<AuthContext>& pContext, AuthenticateStepsWidget* pStepsWidget);
		virtual ~StepChooseCardGui() override;

		virtual void activate() override;
		virtual void deactivate() override;
};

} /* namespace governikus */
