/*!
 * \brief Unit tests for History.
 *
 * All tests ends with _QTEST to be able to identify them later.
 * All original history entry from AusweisApp2 do not have this.
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#include "HistorySettings.h"

#include "TestFileHelper.h"

#include <QtCore>
#include <QtTest>

using namespace governikus;

class test_HistorySettings
	: public QObject
{
	Q_OBJECT
	QScopedPointer<HistorySettings> settings;

	private Q_SLOTS:
		void init()
		{
			QCoreApplication::setOrganizationName(QStringLiteral("dummy"));
			AbstractSettings::mTestDir.clear();
			settings.reset(new HistorySettings());
		}


		void testEnabled()
		{
			bool initial = settings->isEnabled();

			settings->setEnabled(!initial);
			QCOMPARE(settings->isEnabled(), !initial);
			settings->save();

			settings->setEnabled(initial);
			QCOMPARE(settings->isEnabled(), initial);
		}


		void testHistoryEntries()
		{
			QVector<HistoryInfo> initial = settings->getHistoryInfos();
			HistoryInfo info("pSubjectName", "pSubjectUrl", "pUsage", QDateTime(), "pTermOfUsage", "pRequestedData");
			QVector<HistoryInfo> newValue(initial);
			newValue.prepend(info); // new values will be prepended, so that it appears on top

			settings->addHistoryInfo(info);
			QCOMPARE(settings->getHistoryInfos(), newValue);
		}


		void testDeleteHistory()
		{
			HistoryInfo info("pSubjectName", "pSubjectUrl", "pUsage", QDateTime(), "pTermOfUsage", "pRequestedData");
			settings->addHistoryInfo(info);

			QCOMPARE(settings->getHistoryInfos().size(), 1);

			settings->deleteSettings();

			QCOMPARE(settings->getHistoryInfos().size(), 0);
		}


		void testDeleteHistoryFromFile()
		{
			const auto file = AbstractSettings::mTestDir->path() + QStringLiteral("/dummy/Test_settings_HistorySettings.ini");

			HistoryInfo info("pSubjectXYZ", "pSubjectUrlXYZ", "pUsageXYZ", QDateTime(), "pTermOfUsageXYZ", "pRequestedDataXYZ");
			settings->addHistoryInfo(info);
			settings->addHistoryInfo(info);
			settings->addHistoryInfo(info);
			settings->save();
			QVERIFY(QFile::exists(file));

			auto content = TestFileHelper::readFile(file);
			QVERIFY(content.contains("pSubjectXYZ"));
			QVERIFY(content.contains("pSubjectUrlXYZ"));
			QVERIFY(content.contains("pUsageXYZ"));
			QVERIFY(content.contains("pTermOfUsageXYZ"));
			QVERIFY(content.contains("pRequestedDataXYZ"));

			settings->deleteSettings();
			settings->save();

			content = TestFileHelper::readFile(file);
			QVERIFY(!content.contains("pSubjectXYZ"));
			QVERIFY(!content.contains("pSubjectUrlXYZ"));
			QVERIFY(!content.contains("pUsageXYZ"));
			QVERIFY(!content.contains("pTermOfUsageXYZ"));
			QVERIFY(!content.contains("pRequestedDataXYZ"));
		}


};

QTEST_GUILESS_MAIN(test_HistorySettings)
#include "test_HistorySettings.moc"
