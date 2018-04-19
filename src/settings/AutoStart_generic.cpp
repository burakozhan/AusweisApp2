/*!
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#include "AutoStart.h"

#include <QLoggingCategory>


using namespace governikus;

Q_DECLARE_LOGGING_CATEGORY(settings)

bool AutoStart::enabled()
{
	return false;
}


void AutoStart::set(bool pEnabled)
{
	if (pEnabled)
	{
		qCDebug(settings) << "Autostart not supported on this system";
	}
}
