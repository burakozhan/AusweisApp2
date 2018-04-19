/*!
 * \copyright Copyright (c) 2016-2018 Governikus GmbH & Co. KG, Germany
 */

#include "LanguageLoader.h"
#include "LogFilesDialog.h"
#include "ui_LogFilesDialog.h"

#include "generic/HelpAction.h"
#include "LogHandler.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfoList>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTextCursor>

Q_DECLARE_LOGGING_CATEGORY(gui)

using namespace governikus;


LogFilesDialog::LogFilesDialog(QWidget* pParent)
	: QDialog(pParent)
	, mUi(new Ui::LogFilesDialog)
{
	mUi->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);
	setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
	setWindowTitle(QCoreApplication::applicationName() + QStringLiteral(" - ") + tr("Log"));

	connect(mUi->saveButton, &QAbstractButton::clicked, this, &LogFilesDialog::onSaveButtonClicked);
	connect(mUi->deleteButton, &QAbstractButton::clicked, this, &LogFilesDialog::onDeleteButtonClicked);
	connect(mUi->closeButton, &QAbstractButton::clicked, this, &LogFilesDialog::close);

	init();
}


LogFilesDialog::~LogFilesDialog()
{
}


void LogFilesDialog::init()
{
	mUi->logFilesComboBox->clear();

	auto model = new QStandardItemModel(this);
	mUi->logFilesComboBox->setModel(model);

	const auto& otherLogs = LogHandler::getInstance().getOtherLogfiles();
	QList<QStandardItem*> items;
	items.reserve(otherLogs.size() + 1);
	items << new QStandardItem(tr("Current log"));
	for (const auto& entry : otherLogs)
	{
		auto date = LogHandler::getFileDate(entry);
		auto item = new QStandardItem(LanguageLoader::getInstance().getUsedLocale().toString(date, tr("dd.MM.yyyy hh:mm:ss AP")));
		item->setData(date, Qt::UserRole);
		item->setData(entry.absoluteFilePath());
		items << item;
	}

	mUi->logFilesComboBox->setEnabled(!otherLogs.isEmpty());
	mUi->deleteButton->setEnabled(!otherLogs.isEmpty());

	model->invisibleRootItem()->appendRows(items);
	model->setSortRole(Qt::UserRole);
	model->sort(0, Qt::DescendingOrder);

	connect(mUi->logFilesComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LogFilesDialog::onCurrentIndexChanged);

	mUi->plainTextEdit->clear();
	appendLoggingDump(QString::fromUtf8(LogHandler::getInstance().getBacklog()).toHtmlEscaped());
	mUi->plainTextEdit->moveCursor(QTextCursor::Start);
	connect(&LogHandler::getInstance(), &LogHandler::fireLog, this, &LogFilesDialog::doLogMsg);
}


void LogFilesDialog::appendLoggingDump(const QString& pLog)
{
	mUi->plainTextEdit->appendHtml(QStringLiteral("<pre>%1</pre>").arg(pLog));
}


void LogFilesDialog::doLogMsg(const QString& pMsg)
{
	appendLoggingDump(pMsg.toHtmlEscaped());
}


void LogFilesDialog::onCurrentIndexChanged(int pIndex)
{
	mUi->plainTextEdit->clear();
	if (pIndex == 0)
	{
		connect(&LogHandler::getInstance(), &LogHandler::fireLog, this, &LogFilesDialog::doLogMsg);

		appendLoggingDump(QString::fromUtf8(LogHandler::getInstance().getBacklog()).toHtmlEscaped());
		mUi->plainTextEdit->moveCursor(QTextCursor::Start);
	}
	else
	{
		disconnect(&LogHandler::getInstance(), &LogHandler::fireLog, this, &LogFilesDialog::doLogMsg);

		QFile file(mUi->logFilesComboBox->itemData(pIndex, Qt::UserRole + 1).toString());
		if (file.size() < 3145728)
		{
			if (file.open(QIODevice::ReadOnly))
			{
				appendLoggingDump(QString::fromUtf8(file.readAll()));
			}
			else
			{
				appendLoggingDump(tr("File could not be opened: ") + file.fileName());
			}
		}
		else
		{
			appendLoggingDump(tr("File is larger than 3 MB and can not be displayed: ") + file.fileName());
		}
	}
}


void LogFilesDialog::onDeleteButtonClicked()
{
	QMessageBox box(this);
	box.setWindowTitle(tr("Delete log files"));
	box.setWindowModality(Qt::ApplicationModal);
	box.setIcon(QMessageBox::Question);
	box.setWindowFlags(box.windowFlags() & ~Qt::WindowContextHelpButtonHint);
	box.setText(tr("Do you really want to delete all old log files?"));
	box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	box.button(QMessageBox::Yes)->setFocus();

	if (box.exec() == QMessageBox::Yes)
	{
		LogHandler::getInstance().removeOtherLogfiles();
		init();
	}
}


void LogFilesDialog::onSaveButtonClicked()
{
	const int index = mUi->logFilesComboBox->currentIndex();

	QString source;
	if (index != 0)
	{
		source = mUi->logFilesComboBox->itemData(index, Qt::UserRole + 1).toString();
	}

	saveLogFile(this, source);
}


bool LogFilesDialog::eventFilter(QObject* pObject, QEvent* pEvent)
{
	if (pEvent->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(pEvent);
		if (keyEvent->key() == Qt::Key_F1)
		{
			HelpAction::openContextHelp();
			return true;
		}
	}
	return QDialog::eventFilter(pObject, pEvent);
}


void LogFilesDialog::changeEvent(QEvent* pEvent)
{
	if (pEvent->type() == QEvent::LanguageChange)
	{
		mUi->retranslateUi(this);
	}
	QWidget::changeEvent(pEvent);
}


void LogFilesDialog::saveLogFile(QWidget* pParent, const QString& pSource)
{
	const QDateTime creationDateTime = pSource.isEmpty() ? LogHandler::getInstance().getCurrentLogfileDate() : LogHandler::getFileDate(pSource);

	QString filename = QStringLiteral("AusweisApp2.%1.log").arg(creationDateTime.toString(QStringLiteral("yyyy-MM-dd_HH-mm")));
	filename = QFileDialog::getSaveFileName(pParent,
			QCoreApplication::applicationName() + QStringLiteral(" - ") + tr("Save"),
			QDir::homePath() + QLatin1Char('/') + filename,
#ifndef Q_OS_MACOS
			QStringLiteral("*.log"));
#else
			QStringLiteral("*.log"), nullptr, QFileDialog::DontUseNativeDialog);
#endif
	if (!filename.isEmpty()) // if user does not select "cancel"
	{
		if (!filename.endsWith(QLatin1String(".log"), Qt::CaseSensitivity::CaseInsensitive)
				&& !filename.endsWith(QLatin1String(".txt"), Qt::CaseSensitivity::CaseInsensitive))
		{
			filename += QStringLiteral(".log");
		}

		qCDebug(gui) << "File location:" << filename;

		if (QFile::exists(filename))
		{
			bool deleted = QFile::remove(filename);
			qCDebug(gui) << "Delete file location:" << deleted;
		}

		bool copied = pSource.isEmpty() ? LogHandler::getInstance().copy(filename) : QFile::copy(pSource, filename);
		qCDebug(gui) << "Copy log to file location:" << copied;
		if (!copied)
		{
			QMessageBox box(pParent);
			box.setWindowTitle(QApplication::applicationName() + QStringLiteral(" - ") + tr("File error"));
			box.setWindowModality(Qt::ApplicationModal);
			box.setIcon(QMessageBox::Warning);
			box.setWindowFlags(box.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			box.setText(tr("An error occurred while saving the file."));
			box.setStandardButtons(QMessageBox::Ok);
			box.button(QMessageBox::Ok)->setFocus();
			box.exec();
		}
	}
}
