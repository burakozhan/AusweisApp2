/*!
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */


#include "AbstractSettings.h"

#include <QCoreApplication>
#ifdef Q_OS_MACOS
	#include <QFileInfo>
#endif
#ifdef Q_OS_ANDROID
	#include <QtAndroid>
#endif

using namespace governikus;

#ifndef QT_NO_DEBUG
QSharedPointer<QTemporaryDir> AbstractSettings::mTestDir;
#endif

void AbstractSettings::createLegacyFileMapping()
{
#ifdef Q_OS_MACOS
	static bool checked = false;
	if (checked)
	{
		return;
	}

	QFileInfo oldInfo(QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).fileName());
	oldInfo.setCaching(false);
	QFileInfo newInfo(QSettings().fileName());
	newInfo.setCaching(false);

	if (!oldInfo.isSymLink())
	{
		if (oldInfo.exists() && !newInfo.exists())
		{
			QFile(oldInfo.filePath()).rename(newInfo.filePath());
		}

		if (!oldInfo.exists())
		{
			QFile(newInfo.filePath()).link(oldInfo.filePath());
		}
	}

	checked = true;
#endif
}


AbstractSettings::AbstractSettings()
{
}


AbstractSettings::~AbstractSettings()
{
}


QSharedPointer<QSettings> AbstractSettings::getStore()
{
	createLegacyFileMapping();
#ifndef QT_NO_DEBUG
	if (QCoreApplication::applicationName().startsWith(QLatin1String("Test")))
	{
		if (mTestDir.isNull())
		{
			mTestDir.reset(new QTemporaryDir);
			Q_ASSERT(mTestDir->isValid());
		}
		QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, mTestDir->path());
		return QSharedPointer<QSettings>(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName()));
	}
#endif
	return QSharedPointer<QSettings>(new QSettings());
}


bool AbstractSettings::appIsBackgroundService() const
{
#ifdef Q_OS_ANDROID
	if (QtAndroid::androidService().isValid())
	{
		return true;
	}
#endif
	return false;
}
