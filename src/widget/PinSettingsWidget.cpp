/*!
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#include "PinSettingsWidget.h"

#include "Env.h"
#include "generic/PasswordEdit.h"
#include "RandomPinDialog.h"
#include "ReaderConfiguration.h"
#include "ReaderInfo.h"
#include "ReaderManager.h"
#include "SmartCardDefinitions.h"
#include "ui_PinSettingsWidget.h"

#include <QLoggingCategory>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>

Q_DECLARE_LOGGING_CATEGORY(gui)

using namespace governikus;

PinSettingsWidget::PinSettingsWidget(QWidget* pParent)
	: QWidget(pParent)
	, mUi(new Ui::PinSettingsWidget())
	, mMode(Mode::Normal)
	, mRetryCounter(3)
	, mPinDeactivated(false)
	, mPinButtonEnabled(false)
	, mPinSettingsInfoTitle()
	, mPinSettingsInfoDescription()
	, mRandomPinDialog()
{
	mUi->setupUi(this);

	QRegularExpression onlyNumbersExpression(QStringLiteral("[0-9]*"));

	mUi->canEdit->setMaxLength(6);
	mUi->canEdit->configureValidation(onlyNumbersExpression, tr("Only digits (0-9) are allowed."));
	connect(mUi->canEdit, &PasswordEdit::textEdited, this, &PinSettingsWidget::onCanTextEdited);

	mUi->oldPinEdit->setMaxLength(6);
	mUi->oldPinEdit->configureValidation(onlyNumbersExpression, tr("Only digits (0-9) are allowed."));
	connect(mUi->oldPinEdit, &PasswordEdit::textEdited, this, &PinSettingsWidget::onOldPinTextEdited);
	connect(mUi->oldPinEdit, &PasswordEdit::fireBackspacePressedAndEmpty, this, [this] {
				if (mUi->canEdit->isVisible())
				{
					mUi->canEdit->removeLastCharacter();
					mUi->canEdit->setFocus();
				}
			});

	mUi->newPinEdit->setMaxLength(6);
	mUi->newPinEdit->configureValidation(onlyNumbersExpression, tr("Only digits (0-9) are allowed."));
	connect(mUi->newPinEdit, &PasswordEdit::textEdited, this, &PinSettingsWidget::onNewPinTextEdited);
	connect(mUi->newPinEdit, &PasswordEdit::fireBackspacePressedAndEmpty, this, [this] {
				mUi->oldPinEdit->removeLastCharacter();
				mUi->oldPinEdit->setFocus();
			});

	mUi->repeatNewPinEdit->setMaxLength(6);
	mUi->repeatNewPinEdit->configureValidation(onlyNumbersExpression, tr("Only digits (0-9) are allowed."));
	connect(mUi->repeatNewPinEdit, &PasswordEdit::textEdited, this, &PinSettingsWidget::onRepeatNewPinTextEdited);
	connect(mUi->repeatNewPinEdit, &PasswordEdit::fireBackspacePressedAndEmpty, this, [this] {
				mUi->newPinEdit->removeLastCharacter();
				mUi->newPinEdit->setFocus();
			});

	mUi->pukEdit->setMaxLength(10);
	mUi->pukEdit->configureValidation(onlyNumbersExpression, tr("Only digits (0-9) are allowed."));
	connect(mUi->pukEdit, &PasswordEdit::textEdited, this, &PinSettingsWidget::onPukTextEdited);

	mUi->canRandomPinButton->setIcon(QPixmap(QStringLiteral(":/images/randompin/screen_keyboard.png")));
	mUi->canRandomPinButton->setIconSize(QSize(44, 26));
	connect(mUi->canRandomPinButton, &QAbstractButton::clicked, this, &PinSettingsWidget::onRandomPinButtonClicked);

	mUi->oldRandomPinButton->setIcon(QPixmap(QStringLiteral(":/images/randompin/screen_keyboard.png")));
	mUi->oldRandomPinButton->setIconSize(QSize(44, 26));
	connect(mUi->oldRandomPinButton, &QAbstractButton::clicked, this, &PinSettingsWidget::onRandomPinButtonClicked);

	mUi->newRandomPinButton->setIcon(QPixmap(QStringLiteral(":/images/randompin/screen_keyboard.png")));
	mUi->newRandomPinButton->setIconSize(QSize(44, 26));
	connect(mUi->newRandomPinButton, &QAbstractButton::clicked, this, &PinSettingsWidget::onRandomPinButtonClicked);

	mUi->repeatNewRandomPinButton->setIcon(QPixmap(QStringLiteral(":/images/randompin/screen_keyboard.png")));
	mUi->repeatNewRandomPinButton->setIconSize(QSize(44, 26));
	connect(mUi->repeatNewRandomPinButton, &QAbstractButton::clicked, this, &PinSettingsWidget::onRandomPinButtonClicked);

	mUi->pukRandomPinButton->setIcon(QPixmap(QStringLiteral(":/images/randompin/screen_keyboard.png")));
	mUi->pukRandomPinButton->setIconSize(QSize(44, 26));
	connect(mUi->pukRandomPinButton, &QAbstractButton::clicked, this, &PinSettingsWidget::onRandomPukButtonClicked);
}


PinSettingsWidget::~PinSettingsWidget()
{
}


void PinSettingsWidget::setInProgress(bool pInProgress)
{
	if (pInProgress)
	{
		if (mUi->stackedWidget->currentWidget() == mUi->changePinComfortPage)
		{
			mUi->changePinComfortDetailsStackedWidget->setCurrentWidget(mUi->changePinComfortDetailsInProgressPage);
			mPinSettingsInfoDescription = mUi->changePinComfortInProgressLabel->text();
		}
		else if (mUi->stackedWidget->currentWidget() == mUi->changePinBasicPage)
		{
			mUi->canEdit->setEnabled(false);
			mUi->oldPinEdit->setEnabled(false);
			mUi->newPinEdit->setEnabled(false);
			mUi->repeatNewPinEdit->setEnabled(false);
			mUi->canRandomPinButton->setEnabled(false);
			mUi->oldRandomPinButton->setEnabled(false);
			mUi->newRandomPinButton->setEnabled(false);
			mUi->repeatNewRandomPinButton->setEnabled(false);
		}
	}
}


QString PinSettingsWidget::getCan() const
{
	return mUi->canEdit->text();
}


QString PinSettingsWidget::getPin() const
{
	return mUi->oldPinEdit->text();
}


QString PinSettingsWidget::getPuk() const
{
	return mUi->pukEdit->text();
}


QString PinSettingsWidget::getNewPin() const
{
	return mUi->newPinEdit->text();
}


void PinSettingsWidget::setMode(PinSettingsWidget::Mode pMode)
{
	mMode = pMode;
}


QString PinSettingsWidget::getButtonText() const
{
	return mRetryCounter == 0 && !mPinDeactivated ? tr("Enter PUK") : tr("Change PIN");
}


QVector<ReaderInfo> PinSettingsWidget::getReaderWithNPA(const QVector<ReaderInfo>& pReaderInfos)
{
	QVector<ReaderInfo> readersWithNPA;
	for (const ReaderInfo& readerInfo : pReaderInfos)
	{
		if (readerInfo.hasEidCard())
		{
			readersWithNPA += readerInfo;
		}
	}

	return readersWithNPA;
}


void PinSettingsWidget::updateReadersWithoutNPA(const QVector<ReaderInfo>& pReaderInfos)
{
	mMode = Mode::Normal;
	mUi->headerStackedWidget->setCurrentWidget(mUi->errorNoNpaHeaderPage);

	bool readerWithInsufficientApduLength = false;
	for (const auto& readerInfo : pReaderInfos)
	{
		if (!readerInfo.sufficientApduLength())
		{
			readerWithInsufficientApduLength = true;
		}
	}

	bool basicReaderPage = false;
	if (pReaderInfos.size() == 1)
	{
		const ReaderInfo& readerInfo = pReaderInfos.at(0);
		if (readerInfo.isBasicReader())
		{
			setupPinBasicPage(readerInfo);
			basicReaderPage = true;
		}
		else
		{
			QPixmap pixmap(readerInfo.getReaderConfigurationInfo().getIcon()->lookupPath());
			mUi->noNpaLabel->setPixmap(pixmap.scaledToWidth(SCALEWIDTH, Qt::SmoothTransformation));
		}

		if (readerWithInsufficientApduLength)
		{
			mUi->headerStackedWidget->setCurrentWidget(mUi->errorInsufficientApduLengthSingle);
		}
	}
	else
	{
		QPixmap pixmap(ReaderConfiguration::getMultipleReaderIconPath());
		mUi->noNpaLabel->setPixmap(pixmap.scaledToWidth(250, Qt::SmoothTransformation));

		if (readerWithInsufficientApduLength)
		{
			mUi->headerStackedWidget->setCurrentWidget(mUi->errorInsufficientApduLengthMulti);
		}
	}

	mUi->stackedWidget->setCurrentWidget(basicReaderPage ? mUi->changePinBasicPage : mUi->errorNoNpaPage);
	mUi->repeatNewPinEdit->setDigitFieldInvalid(false, tr("PIN correct."));
}


void PinSettingsWidget::setUseScreenKeyboard(bool pUseScreenKeyboard)
{
	mUi->canRandomPinButton->setVisible(pUseScreenKeyboard);
	mUi->oldRandomPinButton->setVisible(pUseScreenKeyboard);
	mUi->newRandomPinButton->setVisible(pUseScreenKeyboard);
	mUi->repeatNewRandomPinButton->setVisible(pUseScreenKeyboard);
	mUi->pukRandomPinButton->setVisible(pUseScreenKeyboard);
}


bool PinSettingsWidget::getPinButtonEnabled() const
{
	return mPinButtonEnabled;
}


void PinSettingsWidget::fillInfoDescription(const QString& pTitle, const QString& pMessage)
{
	mPinSettingsInfoTitle = pTitle;
	mPinSettingsInfoDescription = pMessage;
}


bool PinSettingsWidget::updateReadersForOneNPA(const ReaderInfo& pReaderInfo)
{
	mRetryCounter = pReaderInfo.getRetryCounter();
	mPinDeactivated = pReaderInfo.isPinDeactivated();

	if (mMode == Mode::AfterPinChange)
	{
		setupPinSuccessfullyChangedPage(pReaderInfo);
		mUi->headerStackedWidget->setCurrentWidget(mUi->pinSuccessHeaderPage);
		mUi->stackedWidget->setCurrentWidget(mUi->pinSuccessPage);
		return true;
	}

	if (mPinDeactivated)
	{
		if (pReaderInfo.isBasicReader())
		{
			setupPinBasicPage(pReaderInfo);
			mUi->stackedWidget->setCurrentWidget(mUi->changePinBasicPage);
		}
		else
		{
			QPixmap pixmap(pReaderInfo.getReaderConfigurationInfo().getIconWithNPA()->lookupPath());
			mUi->deactivatedReaderImageLabel->setPixmap(pixmap.scaledToWidth(SCALEWIDTH, Qt::SmoothTransformation));
			mUi->stackedWidget->setCurrentWidget(mUi->errorPinDeactivatedPage);
		}

		mUi->headerStackedWidget->setCurrentWidget(mUi->errorPinDeactivatedHeaderPage);
		return false;
	}

	if (pReaderInfo.isBasicReader())
	{
		setupPinBasicPage(pReaderInfo);
		mUi->stackedWidget->setCurrentWidget(mUi->changePinBasicPage);
	}
	else
	{
		setupPinComfortPage(pReaderInfo);
		mUi->stackedWidget->setCurrentWidget(mUi->changePinComfortPage);
	}

	return !pReaderInfo.isBasicReader();
}


void PinSettingsWidget::updateReaders()
{
	const ReaderManager& readerManager = *Env::getSingleton<ReaderManager>();
	if (readerManager.getReaderInfos().isEmpty())
	{
		QPixmap pixmap(ReaderConfiguration::getNoReaderFoundIconPath());
		mUi->noReaderLabel->setPixmap(pixmap.scaledToWidth(SCALEWIDTH, Qt::SmoothTransformation));
		mUi->headerStackedWidget->setCurrentWidget(mUi->errorNoReaderHeaderPage);
		mUi->stackedWidget->setCurrentWidget(mUi->errorNoReaderPage);
		return;
	}

	QVector<ReaderInfo> readersWithNPA = getReaderWithNPA(readerManager.getReaderInfos());
	mRetryCounter = 3;
	bool enableButton = false;

	if (readersWithNPA.size() == 0)
	{
		updateReadersWithoutNPA(readerManager.getReaderInfos(ReaderFilter::UniqueReaderTypes));
	}
	else if (readersWithNPA.size() == 1)
	{
		enableButton = updateReadersForOneNPA(readersWithNPA.at(0));
	}
	else
	{
		mMode = Mode::Normal;
		QPixmap pixmap(ReaderConfiguration::getMultipleReaderIconPath());
		mUi->multipleReaderLabel->setPixmap(pixmap.scaledToWidth(SCALEWIDTH, Qt::SmoothTransformation));
		mUi->headerStackedWidget->setCurrentWidget(mUi->errorMultipleNpasHeaderPage);
		mUi->stackedWidget->setCurrentWidget(mUi->errorMultipleNpasPage);
	}

	mPinButtonEnabled = enableButton;
	Q_EMIT firePinButtonEnabledUpdated(mPinButtonEnabled);
}


void PinSettingsWidget::onBackspacePressedOnApply()
{
	mUi->repeatNewPinEdit->removeLastCharacter();
	mUi->repeatNewPinEdit->setFocus();
	onRepeatNewPinTextEdited();
}


void PinSettingsWidget::paintEvent(QPaintEvent*)
{
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}


void PinSettingsWidget::showEvent(QShowEvent* pEvent)
{
	connect(&ReaderManager::getInstance(), &ReaderManager::fireReaderEvent, this, &PinSettingsWidget::updateReaders);
	connect(&ReaderManager::getInstance(), &ReaderManager::fireCardRetryCounterChanged, this, &PinSettingsWidget::updateReaders);

	updateReaders();
	QWidget::showEvent(pEvent);
}


void PinSettingsWidget::hideEvent(QHideEvent* pEvent)
{
	disconnect(&ReaderManager::getInstance(), &ReaderManager::fireReaderEvent, this, &PinSettingsWidget::updateReaders);
	disconnect(&ReaderManager::getInstance(), &ReaderManager::fireCardRetryCounterChanged, this, &PinSettingsWidget::updateReaders);

	QWidget::hideEvent(pEvent);

	// Reset the mode, so that the next time the users sees this widget, it doesn't show the state of the previous
	// action.
	mMode = Mode::Normal;

	if (mRandomPinDialog && mRandomPinDialog->isVisible())
	{
		// close the PinPad in case the tab is hidden,
		// e.g. an authentication was started, so the PIN change is aborted.
		mRandomPinDialog->reject();
	}
}


void PinSettingsWidget::onScanButtonClicked()
{
	ReaderManager::getInstance().startScanAll();
	QTimer::singleShot(3000, this, &PinSettingsWidget::onScanTimeout);
}


void PinSettingsWidget::onScanTimeout()
{
}


void PinSettingsWidget::onUiFinished(const QString& pReaderName)
{
	qDebug() << "Set reader name" << pReaderName;
	updateReaders();
}


void PinSettingsWidget::onCanTextEdited(const QString& pText)
{
	if (!pText.isNull() && !pText.isEmpty())
	{
		mUi->canEdit->setText(pText);
	}

	if (isCanFieldVisible())
	{
		if (mUi->canEdit->text().length() == 6)
		{
			mUi->oldPinEdit->setEnabled(true);
			mUi->oldRandomPinButton->setEnabled(true);

			QTimer::singleShot(300, this, &PinSettingsWidget::focusPIN);
		}
		else
		{
			mUi->oldPinEdit->setEnabled(false);
			mUi->oldRandomPinButton->setEnabled(false);
		}
	}
	else
	{
		mUi->oldPinEdit->setEnabled(true);
		mUi->oldRandomPinButton->setEnabled(true);
		QTimer::singleShot(300, this, &PinSettingsWidget::focusPIN);
	}

	mUi->oldPinEdit->clear();
	onOldPinTextEdited();
}


void PinSettingsWidget::onOldPinTextEdited(const QString& pText)
{
	if (!pText.isNull() && !pText.isEmpty())
	{
		mUi->oldPinEdit->setText(pText);
	}

	bool enable = mUi->oldPinEdit->text().length() >= 5;
	mUi->newPinEdit->setEnabled(enable);
	mUi->newRandomPinButton->setEnabled(enable);

	if (mUi->oldPinEdit->text().length() == 6)
	{
		mUi->newPinEdit->setFocus();
		mUi->newPinEdit->setCursorPosition(0);
	}

	mUi->newPinEdit->clear();
	onNewPinTextEdited();
}


void PinSettingsWidget::onNewPinTextEdited(const QString& pText)
{
	if (!pText.isNull() && !pText.isEmpty())
	{
		mUi->newPinEdit->setText(pText);
	}

	bool enable = mUi->newPinEdit->text().length() == 6;
	mUi->repeatNewPinEdit->setEnabled(enable);
	mUi->repeatNewRandomPinButton->setEnabled(enable);
	if (mUi->newPinEdit->text().length() == 6)
	{
		mUi->repeatNewPinEdit->setFocus();
		mUi->repeatNewPinEdit->setCursorPosition(0);
	}

	mUi->repeatNewPinEdit->clear();
	onRepeatNewPinTextEdited();
}


void PinSettingsWidget::onRepeatNewPinTextEdited(const QString& pText)
{
	if (!pText.isNull() && !pText.isEmpty())
	{
		mUi->repeatNewPinEdit->setText(pText);
	}

	if (!mUi->repeatNewPinEdit->isEnabled() || mUi->newPinEdit->text().startsWith(mUi->repeatNewPinEdit->text()) || mUi->repeatNewPinEdit->text().length() != 6)
	{
		mUi->repeatNewPinEdit->setDigitFieldInvalid(false, QString());

		bool inputOk = mUi->repeatNewPinEdit->text().length() == 6 && mUi->repeatNewPinEdit->text() == mUi->newPinEdit->text();
		mPinButtonEnabled = inputOk;
		Q_EMIT firePinButtonEnabledUpdated(mPinButtonEnabled);
	}
	else
	{
		QString invalidMessage = tr("The PIN in the field \"%1\" does not match the PIN in the field \"%2\".").arg(mUi->repeatNewPinEditLabel->text().replace(QStringLiteral(":"), QLatin1String("")), mUi->newPinEditLabel->text().replace(QStringLiteral(":"), QLatin1String("")));
		mUi->repeatNewPinEdit->setDigitFieldInvalid(true, invalidMessage);
	}

}


void PinSettingsWidget::onPukTextEdited(const QString& pText)
{
	if (!pText.isNull() && !pText.isEmpty())
	{
		mUi->pukEdit->setText(pText);
	}
	mPinButtonEnabled = mUi->pukEdit->text().length() == 10;
	Q_EMIT firePinButtonEnabledUpdated(mPinButtonEnabled);
}


void PinSettingsWidget::setupPinBasicPage(const ReaderInfo& pReaderInfo)
{
	mUi->canEdit->clear();
	mUi->oldPinEdit->clear();
	mUi->newPinEdit->clear();
	mUi->repeatNewPinEdit->clear();
	mUi->pukEdit->clear();

	bool hasCard = pReaderInfo.hasEidCard();

	QPixmap pixmap;
	if (hasCard)
	{
		pixmap = pReaderInfo.getReaderConfigurationInfo().getIconWithNPA()->lookupPath();
		mUi->basicReaderImageLabel->setAccessibleName(tr("card inserted"));
	}
	else
	{
		pixmap = pReaderInfo.getReaderConfigurationInfo().getIcon()->lookupPath();
		mUi->basicReaderImageLabel->setAccessibleName(tr("no card inserted"));
	}

	mUi->basicReaderImageLabel->setPixmap(pixmap.scaledToWidth(SCALEWIDTH, Qt::SmoothTransformation));

	bool canEditVisible = false;
	bool pukEditVisible = false;

	if (hasCard)
	{
		setupChangePinHeader(pReaderInfo.getRetryCounter(), true);

		switch (pReaderInfo.getRetryCounter())
		{
			case 0:
				pukEditVisible = true;
				break;

			case 1:
				canEditVisible = true;
				break;

			default:
				break;
		}
	}

	if (mPinDeactivated)
	{
		pukEditVisible = false;
		canEditVisible = false;
		hasCard = false;
	}

	mUi->canEditStackedWidget->setCurrentWidget(canEditVisible ? mUi->canEditPage : mUi->noCanEditPage);
	mUi->canEditLabelStackedWidget->setCurrentWidget(canEditVisible ? mUi->canEditLabelPage : mUi->noCanEditLabelPage);

	mUi->basicReaderPukStackedWidget->setCurrentWidget(pukEditVisible ? mUi->basicReaderPukPage : mUi->basicReaderPinPage);

	mUi->canEditLabel->setEnabled(hasCard);
	mUi->oldPinEditLabel->setEnabled(hasCard);
	mUi->newPinEditLabel->setEnabled(hasCard);
	mUi->repeatNewPinEditLabel->setEnabled(hasCard);
	mUi->canEdit->setEnabled(hasCard);
	mUi->oldPinEdit->setEnabled(hasCard);
	mUi->newPinEdit->setEnabled(hasCard);
	mUi->repeatNewPinEdit->setEnabled(hasCard);
	mUi->canRandomPinButton->setEnabled(hasCard);
	mUi->oldRandomPinButton->setEnabled(hasCard);
	mUi->newRandomPinButton->setEnabled(hasCard);
	mUi->repeatNewRandomPinButton->setEnabled(hasCard);

	if (hasCard)
	{
		if (isCanFieldVisible())
		{
			QTimer::singleShot(300, this, &PinSettingsWidget::focusCAN);
		}
		else if (pukEditVisible)
		{
			QTimer::singleShot(300, this, &PinSettingsWidget::focusPUK);
		}

		if (!pukEditVisible)
		{
			onCanTextEdited();
		}
	}
}


void PinSettingsWidget::focusPUK()
{
	mUi->pukEdit->setFocus();
	mUi->pukEdit->setCursorPosition(0);
}


void PinSettingsWidget::focusPIN()
{
	mUi->oldPinEdit->setFocus();
	mUi->oldPinEdit->setCursorPosition(0);
}


void PinSettingsWidget::focusCAN()
{
	mUi->canEdit->setFocus();
	mUi->canEdit->setCursorPosition(0);
}


void PinSettingsWidget::setupPinComfortPage(const ReaderInfo& pReaderInfo)
{
	QPixmap pixmap(pReaderInfo.getReaderConfigurationInfo().getIconWithNPA()->lookupPath());
	mUi->comfortReaderImageLabel->setPixmap(pixmap.scaledToWidth(SCALEWIDTH, Qt::SmoothTransformation));
	setupChangePinHeader(pReaderInfo.getRetryCounter(), false);

	switch (pReaderInfo.getRetryCounter())
	{
		case 0:
			mUi->changePinComfortDetailsStackedWidget->setCurrentWidget(mUi->changePinComfortPukDetailsPage);
			break;

		case 1:
			mUi->changePinComfortDetailsStackedWidget->setCurrentWidget(mUi->changePinComfortCanDetailsPage);
			break;

		default:
			mUi->changePinComfortDetailsStackedWidget->setCurrentWidget(mUi->changePinComfortDetailsPage);
			break;
	}
}


void PinSettingsWidget::setupPinSuccessfullyChangedPage(const ReaderInfo& pReaderInfo)
{
	QPixmap pixmap(pReaderInfo.getReaderConfigurationInfo().getIconWithNPA()->lookupPath());
	mUi->pinSuccessReaderImageLabel->setPixmap(pixmap.scaledToWidth(SCALEWIDTH, Qt::SmoothTransformation));
}


void PinSettingsWidget::setupChangePinHeader(int pRetryCounter, bool pIsBasicReader)
{
	switch (pRetryCounter)
	{
		case 0:
			mUi->headerStackedWidget->setCurrentWidget(mUi->changePinPukHeaderPage);
			break;

		case 1:
			mUi->headerStackedWidget->setCurrentWidget(mUi->changePinWithCanHeaderPage);
			break;

		default:
			if (mMode == Mode::AfterPinUnblock)
			{
				mUi->headerStackedWidget->setCurrentWidget(mUi->pinUnblockedHeaderPage);
			}
			else
			{
				if (pIsBasicReader)
				{
					mUi->headerStackedWidget->setCurrentWidget(mUi->changePinBasicHeaderPage);
				}
				else
				{
					mUi->headerStackedWidget->setCurrentWidget(mUi->changePinComfortHeaderPage);
				}
			}
			break;
	}
}


bool PinSettingsWidget::isCanFieldVisible() const
{
	return mUi->canEditStackedWidget->currentWidget() == mUi->canEditPage;
}


void PinSettingsWidget::onRandomPinButtonClicked()
{
	const auto& readersWithNpa = getReaderWithNPA(ReaderManager::getInstance().getReaderInfos());
	const auto& selectedReaderName = readersWithNpa.size() == 1 ? readersWithNpa.at(0).getName() : QString();
	mRandomPinDialog = new RandomPinDialog(6, selectedReaderName, this);
	if (mRandomPinDialog->exec() == QDialog::Accepted && !mRandomPinDialog->getPin().isEmpty())
	{
		QToolButton* pinButton = qobject_cast<QToolButton*>(sender());
		if (pinButton == nullptr)
		{
			qCCritical(gui) << "sender == nullptr";
		}
		else if (pinButton->objectName() == QLatin1String("canRandomPinButton"))
		{
			onCanTextEdited(mRandomPinDialog->getPin());
		}
		else if (pinButton->objectName() == QLatin1String("oldRandomPinButton"))
		{
			onOldPinTextEdited(mRandomPinDialog->getPin());
		}
		else if (pinButton->objectName() == QLatin1String("newRandomPinButton"))
		{
			onNewPinTextEdited(mRandomPinDialog->getPin());
		}
		else if (pinButton->objectName() == QLatin1String("repeatNewRandomPinButton"))
		{
			onRepeatNewPinTextEdited(mRandomPinDialog->getPin());
		}
		else if (pinButton->objectName() == QLatin1String("pukRandomPinButton"))
		{
			onPukTextEdited(mRandomPinDialog->getPin());
		}
	}
}


void PinSettingsWidget::onRandomPukButtonClicked()
{
	const auto& readersWithNpa = getReaderWithNPA(ReaderManager::getInstance().getReaderInfos());
	const auto& selectedReaderName = readersWithNpa.size() == 1 ? readersWithNpa.at(0).getName() : QString();
	mRandomPinDialog = new RandomPinDialog(10, selectedReaderName, this);
	if (mRandomPinDialog->exec() == QDialog::Accepted && !mRandomPinDialog->getPin().isEmpty())
	{
		onPukTextEdited(mRandomPinDialog->getPin());
	}
}


void PinSettingsWidget::changeEvent(QEvent* pEvent)
{
	if (pEvent->type() == QEvent::LanguageChange)
	{
		mUi->retranslateUi(this);
	}
	QWidget::changeEvent(pEvent);
}
