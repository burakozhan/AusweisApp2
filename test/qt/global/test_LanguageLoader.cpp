/*!
 * \brief Unit tests for \ref LanguageLoader
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#include "LanguageLoader.h"

#include "FileDestination.h"
#include "LogHandler.h"
#include "TestFileHelper.h"

#include <QtTest>

using namespace governikus;

class test_LanguageLoader
	: public QObject
{
	Q_OBJECT

	QTemporaryDir mTranslationDir;

	private Q_SLOTS:
		void initTestCase()
		{
			LogHandler::getInstance().init();
			TestFileHelper::createTranslations(mTranslationDir.path());
			LanguageLoader::getInstance().setPath(mTranslationDir.path());
		}


		void cleanup()
		{
			if (LanguageLoader::getInstance().isLoaded())
			{
				LanguageLoader::getInstance().unload();
			}
			LogHandler::getInstance().resetBacklog();
		}


		void benchmark()
		{
			QBENCHMARK {
				QCOMPARE(LanguageLoader::getInstance().getAvailableLocales().size(), 6);
			}
		}


		void loadTwice()
		{
			QSignalSpy spy(&LogHandler::getInstance(), &LogHandler::fireLog);

			QVERIFY(!LanguageLoader::getInstance().isLoaded());
			LanguageLoader::getInstance().load(QLocale::German);
			QVERIFY(LanguageLoader::getInstance().isLoaded());

			LanguageLoader::getInstance().load(QLocale::German);
			QVERIFY(spy.count() > 0);
			auto param = spy.takeLast();
			QVERIFY(param.at(0).toString().contains("Loader is already in use. You need to unload before you load again..."));
		}


		void load()
		{
			QLocale locale = QLocale::French;
			LanguageLoader::getInstance().load(locale);
			QVERIFY(LanguageLoader::getInstance().isLoaded());
			QVERIFY(!LanguageLoader::getInstance().mTranslatorList.isEmpty());

			LanguageLoader::getInstance().load(QLocale::German);
			QVERIFY(LanguageLoader::getInstance().isLoaded());

			QCOMPARE(LanguageLoader::getInstance().getUsedLocale(), locale);
		}


		void unload()
		{
			QVERIFY(!LanguageLoader::getInstance().isLoaded());

			LanguageLoader::getInstance().unload();
			QVERIFY(!LanguageLoader::getInstance().isLoaded());

			LanguageLoader::getInstance().unload();
			QVERIFY(!LanguageLoader::getInstance().isLoaded());

			LanguageLoader::getInstance().unload();
			QVERIFY(!LanguageLoader::getInstance().isLoaded());

			QVERIFY(LanguageLoader::getInstance().mTranslatorList.isEmpty());
		}


		void isLoaded()
		{
			LanguageLoader::mDefaultLanguage = QLocale::German; // avoid QLocale::C of QLocale::system()
			QVERIFY(!LanguageLoader::getInstance().isLoaded());
			LanguageLoader::getInstance().load();
			QVERIFY(LanguageLoader::getInstance().isLoaded());

			LanguageLoader::getInstance().unload();
			QVERIFY(!LanguageLoader::getInstance().isLoaded());
		}


		void usedLocale()
		{
			QLocale locale = QLocale::German;
			LanguageLoader::getInstance().load(locale);
			QCOMPARE(LanguageLoader::getInstance().getUsedLocale(), locale);

			QVERIFY(!LanguageLoader::getInstance().mTranslatorList.isEmpty());
		}


		void unknownLocale()
		{
			LanguageLoader::getInstance().load(QLocale::Cherokee);
			QVERIFY(!LanguageLoader::getInstance().isLoaded());

			QLocale locale = QLocale::English;
			QCOMPARE(LanguageLoader::getInstance().getUsedLocale(), locale);

			QVERIFY(LanguageLoader::getInstance().mTranslatorList.isEmpty());
		}


		void fallbackLanguage()
		{
			QLocale::Language lang = LanguageLoader::getInstance().getFallbackLanguage();
			QCOMPARE(lang, QLocale::English);


			QSignalSpy spy(&LogHandler::getInstance(), &LogHandler::fireLog);

			LanguageLoader::getInstance().load(QLocale::English);
			QVERIFY(spy.count() > 0);
			auto param = spy.takeAt(2);
			QVERIFY(param.at(0).toString().contains("Using fallback language: "));
		}


		void defaultPath()
		{
			LanguageLoader::getInstance().setPath("dummy");
			QCOMPARE(LanguageLoader::getInstance().getPath(), QString("dummy"));
		}


};

QTEST_GUILESS_MAIN(test_LanguageLoader)
#include "test_LanguageLoader.moc"
