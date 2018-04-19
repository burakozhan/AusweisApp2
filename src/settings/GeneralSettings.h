/*!
 * \brief Contains the definition of the GeneralSettings class.
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "AbstractSettings.h"

#include <QFuture>
#include <QUuid>

class test_GeneralSettings;


namespace governikus
{

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
	#define GENERAL_SETTINGS_DEFAULT_AUTOSTART true
#else
	#define GENERAL_SETTINGS_DEFAULT_AUTOSTART false
#endif


class GeneralSettings
	: public AbstractSettings
{
	Q_OBJECT

	friend class AppSettings;
	friend class ::test_GeneralSettings;
	friend bool operator==(const GeneralSettings& pLeft, const GeneralSettings& pRight);

	private:
		QFuture<bool> mAutoStart;
		QSharedPointer<QSettings> mStoreGeneral;
		QSharedPointer<QSettings> mStoreCommon;

		GeneralSettings();

	public:
		virtual ~GeneralSettings() override;
		virtual void save() override;

		bool isAutoStart() const;
		void setAutoStart(bool pAutoStart);

		const QString getPersistentSettingsVersion() const;

		QString getSkipVersion();
		void skipVersion(const QString& pVersion);

		bool isAutoCloseWindowAfterAuthentication() const;
		void setAutoCloseWindowAfterAuthentication(bool pAutoClose);

		bool isShowSetupAssistant() const;
		void setShowSetupAssistant(bool pShowSetupAssistant);

		bool isRemindUserToClose() const;
		void setRemindUserToClose(bool pRemindUser);

		bool isTransportPinReminder() const;
		void setTransportPinReminder(bool pTransportPinReminder);

		bool isDeveloperMode() const;
		void setDeveloperMode(bool pEnabled);

		bool useSelfAuthTestUri() const;
		void setUseSelfauthenticationTestUri(bool pUse);

		QLocale::Language getLanguage() const;
		void setLanguage(const QLocale::Language pLanguage);

		bool isAutoUpdateCheck() const;
		void setAutoUpdateCheck(bool pAutoUpdateCheck);

		bool isUseScreenKeyboard() const;
		void setUseScreenKeyboard(bool pUseScreenKeyboard);

		QString getLastReaderPluginType() const;
		void setLastReaderPluginType(const QString& pLastReaderPluginType);
};


} /* namespace governikus */
