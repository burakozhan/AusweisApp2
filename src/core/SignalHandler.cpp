/*
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#include "SignalHandler.h"

#include "SingletonHelper.h"

#include <QCoreApplication>
#include <QLoggingCategory>


using namespace governikus;

defineSingleton(SignalHandler)

Q_DECLARE_LOGGING_CATEGORY(system)


SignalHandler::SignalHandler()
	: mInit(false)
	, mAppController()
	, mQuit(false)
{
}


SignalHandler::~SignalHandler()
{
}


SignalHandler& SignalHandler::getInstance()
{
	return *Instance;
}


void SignalHandler::init()
{
	if (!mInit)
	{
#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_MACOS) || defined(Q_OS_FREEBSD)
		initUnix();
#elif defined(Q_OS_WIN) && !defined(Q_OS_WINRT)
		SetConsoleCtrlHandler(PHANDLER_ROUTINE(ctrlHandler), true);
#endif

		mInit = true;
	}
}


void SignalHandler::setController(AppController& pAppController)
{
	mAppController = &pAppController;
}


void SignalHandler::quit()
{
	mQuit = true;

	if (mAppController)
	{
		QMetaObject::invokeMethod(mAppController.data(), "doShutdown", Qt::QueuedConnection);
	}
	else
	{
		QCoreApplication::exit();
	}
}


bool SignalHandler::shouldQuit() const
{
	return mQuit;
}


#if !(defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS))
// A dummy is required for all platform since moc fails to handle some defines.
void SignalHandler::onSignalSocketActivated()
{

}


#endif
