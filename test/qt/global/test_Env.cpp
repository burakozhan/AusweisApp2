/*!
 * \brief Unit tests for \ref Env
 *
 * \copyright Copyright (c) 2017-2018 Governikus GmbH & Co. KG, Germany
 */

#include "Env.h"

#include <QScopedPointer>
#include <QtTest>

using namespace governikus;

namespace
{
class AbstractTestPodInstance
{
	public:
		virtual ~AbstractTestPodInstance()
		{
		}


		virtual QString dummy() = 0;
};

class AbstractTestInstance
{
	Q_GADGET

	public:
		virtual ~AbstractTestInstance()
		{
		}


		virtual QString dummy() = 0;
};

class AbstractTestInstanceImpl
	: public AbstractTestInstance
{
	Q_GADGET

	public:
		virtual QString dummy() override
		{
			return QStringLiteral("impl");
		}


};

class MockedAbstractTestInstance
	: public AbstractTestInstance
{
	Q_GADGET

	public:
		virtual QString dummy() override
		{
			return QStringLiteral("mocked");
		}


};

class TestInstance
{
	Q_GADGET

	protected:
		TestInstance() = default;
		virtual ~TestInstance()
		{
		}


	public:
		static TestInstance& getInstance()
		{
			static TestInstance instance;
			return instance;
		}


		virtual QString something()
		{
			return QStringLiteral("orig");
		}


};

class TestMockedInstance
	: public TestInstance
{
	public:
		virtual QString something() override
		{
			return QStringLiteral("mocked");
		}


};

class TestSharedInstance
	: public QObject
{
	Q_OBJECT

	public:
		virtual QString something()
		{
			return QStringLiteral("orig");
		}


};

class TestMockedSharedInstance
	: public TestSharedInstance
{
	Q_OBJECT

	public:
		virtual QString something() override
		{
			return QStringLiteral("mocked");
		}


};

class TestAbstractUnmanagedInstance
{
	Q_GADGET

	public:
		virtual ~TestAbstractUnmanagedInstance() = default;
		virtual QString something() = 0;
};

class TestUnmanagedInstance
	: public TestAbstractUnmanagedInstance
{
	Q_GADGET

	public:
		virtual QString something() override
		{
			return QStringLiteral("TestUnmanagedInstance");
		}


};


class TestMoveCtorAssign
	: public QObject
{
	Q_OBJECT

	private:
		QString mData;

	public:
		TestMoveCtorAssign(const QString& pData)
			: mData(pData)
		{
		}


		TestMoveCtorAssign(TestMoveCtorAssign&& pCopy)
		{
			mData = std::move(pCopy.mData);
		}


		TestMoveCtorAssign& operator=(TestMoveCtorAssign&& pCopy)
		{
			mData = std::move(pCopy.mData);
			return *this;
		}


		QString data()
		{
			return mData;
		}


};

}

namespace governikus
{

template<> TestAbstractUnmanagedInstance* singleton<TestAbstractUnmanagedInstance>(bool& pTakeOwnership)
{
	pTakeOwnership = false;
	static TestUnmanagedInstance instance;
	return &instance;
}


template<> AbstractTestInstance* singleton<AbstractTestInstance>(bool&)
{
	return new AbstractTestInstanceImpl;
}


template<> AbstractTestInstance* createNewObject<AbstractTestInstance*>()
{
	return new AbstractTestInstanceImpl;
}


template<> AbstractTestInstance* createNewObject<AbstractTestInstance*, QString>(QString&& pStr)
{
	class tmpCtor
		: public AbstractTestInstance
	{
		private:
			const QString mDummy;

		public:
			tmpCtor(const QString& pDummy) : mDummy(pDummy)
			{
			}


			virtual QString dummy() override
			{
				return mDummy;
			}


	};

	return new tmpCtor(pStr);
}


template<> AbstractTestPodInstance* createNewObject<AbstractTestPodInstance*>()
{
	return nullptr;
}


template<> AbstractTestPodInstance* createNewObject<AbstractTestPodInstance*, QString>(QString&&)
{
	return nullptr;
}


}


class test_Env
	: public QObject
{
	Q_OBJECT

	private Q_SLOTS:
		void cleanup()
		{
			Env::clear();
		}


		void checkOverrideMock()
		{
			QCOMPARE(Env::getInstance().mInstancesCreator.size(), 0);

			QScopedPointer<AbstractTestInstance> obj(Env::create<AbstractTestInstance*>());
			QVERIFY(obj);

			const std::function<AbstractTestInstance*()> func = [](){
						return nullptr;
					};
			Env::setCreator<AbstractTestInstance*>(func);

			QCOMPARE(Env::getInstance().mInstancesCreator.size(), 1);
			Env::setCreator<AbstractTestInstance*>(func);
			Env::setCreator<AbstractTestInstance*>(func);
			Env::setCreator<AbstractTestInstance*>(func);
			QCOMPARE(Env::getInstance().mInstancesCreator.size(), 1);

			QCOMPARE(Env::create<AbstractTestInstance*>(), nullptr);
		}


		void mockGet()
		{
			auto orig = Env::getSingleton<TestInstance>();
			QVERIFY(orig);
			QCOMPARE(orig->something(), QLatin1String("orig"));

			TestMockedInstance mock;
			Env::set(TestInstance::staticMetaObject, &mock);
			auto mocked = Env::getSingleton<TestInstance>();
			QVERIFY(mocked);
			QCOMPARE(mocked->something(), QLatin1String("mocked"));

			Env::set(TestInstance::staticMetaObject);
			auto orig_again = Env::getSingleton<TestInstance>();
			QVERIFY(orig_again);
			QCOMPARE(orig_again->something(), QLatin1String("orig"));
		}


		void mockAbstract()
		{
			auto impl = Env::getSingleton<AbstractTestInstance>();
			QVERIFY(impl);
			QCOMPARE(impl->dummy(), QLatin1String("impl"));

			auto impl2 = Env::getSingleton<AbstractTestInstance>();
			QVERIFY(impl == impl2);

			MockedAbstractTestInstance m;
			Env::set(AbstractTestInstance::staticMetaObject, &m);
			auto mocked = Env::getSingleton<AbstractTestInstance>();
			QVERIFY(mocked);
			QVERIFY(mocked != impl);
			QCOMPARE(mocked->dummy(), QLatin1String("mocked"));
		}


		void mockShared()
		{
			auto orig = Env::getShared<TestSharedInstance>();
			QVERIFY(orig);
			QCOMPARE(orig->something(), QLatin1String("orig"));

			auto orig2 = Env::getShared<TestSharedInstance>();
			QVERIFY(orig == orig2);

			auto mock = QSharedPointer<TestMockedSharedInstance>::create();
			Env::setShared(TestSharedInstance::staticMetaObject, mock);
			auto mocked = Env::getShared<TestSharedInstance>();
			QVERIFY(mocked);
			QCOMPARE(mocked->something(), QLatin1String("mocked"));
			QVERIFY(mock == mocked);
			QVERIFY(orig != mocked);

			Env::setShared(TestSharedInstance::staticMetaObject);
			auto orig3 = Env::getShared<TestSharedInstance>();
			QVERIFY(orig != orig3);
			QCOMPARE(orig3->something(), QLatin1String("orig"));
		}


		void getUnmanagedSingleton()
		{
			auto first = Env::getSingleton<TestAbstractUnmanagedInstance>();
			QVERIFY(first);
			QCOMPARE(first->something(), QLatin1String("TestUnmanagedInstance"));

			auto second = Env::getSingleton<TestAbstractUnmanagedInstance>();
			QVERIFY(second);
			QVERIFY(first == second);
		}


		void getMockSingleton()
		{
			MockedAbstractTestInstance m;
			Env::set(AbstractTestInstance::staticMetaObject, &m);
			MockedAbstractTestInstance* mocked = Env::getSingleton<AbstractTestInstance, MockedAbstractTestInstance>();
			QVERIFY(mocked);
			QCOMPARE(mocked->dummy(), QLatin1String("mocked"));

			Env::set(AbstractTestInstance::staticMetaObject);
			mocked = Env::getSingleton<AbstractTestInstance, MockedAbstractTestInstance>();
			QVERIFY(!mocked);


			Env::set(AbstractTestInstance::staticMetaObject, &m);
			Env::set(AbstractTestInstance::staticMetaObject, std::make_shared<MockedAbstractTestInstance>());
			MockedAbstractTestInstance* mocked2 = Env::getSingleton<AbstractTestInstance, MockedAbstractTestInstance>();
			QVERIFY(mocked2);
			QCOMPARE(mocked2->dummy(), QLatin1String("mocked"));

			Env::set(AbstractTestInstance::staticMetaObject);
			AbstractTestInstance* orig = Env::getSingleton<AbstractTestInstance>();
			QVERIFY(orig);
			QCOMPARE(orig->dummy(), QLatin1String("impl"));
			QVERIFY((!Env::getSingleton<AbstractTestInstance, MockedAbstractTestInstance>()));

			Env::set(AbstractTestInstance::staticMetaObject, std::make_shared<MockedAbstractTestInstance>());
			AbstractTestInstance* mocked3 = Env::getSingleton<AbstractTestInstance>();
			QVERIFY(mocked3);
			QCOMPARE(mocked3->dummy(), QLatin1String("mocked"));

			Env::set(AbstractTestInstance::staticMetaObject, std::shared_ptr<void>());
			mocked3 = Env::getSingleton<AbstractTestInstance>();
			QVERIFY(mocked3);
			QCOMPARE(mocked3->dummy(), QLatin1String("impl"));
		}


		void mockCreateNewInstance()
		{
			QScopedPointer<AbstractTestInstance> implOrig(Env::create<AbstractTestInstance*>());
			QVERIFY(implOrig);
			QCOMPARE(implOrig->dummy(), QLatin1String("impl"));
			implOrig.reset();

			std::function<AbstractTestInstance*()> func = [](){
						class tmp
							: public AbstractTestInstanceImpl
						{
							virtual QString dummy() override
							{
								return QStringLiteral("lambda");
							}


						};

						return new tmp;
					};

			Env::setCreator<AbstractTestInstance*>(func);

			QScopedPointer<AbstractTestInstance> impl(Env::create<AbstractTestInstance*>());
			QVERIFY(impl);
			QCOMPARE(impl->dummy(), QLatin1String("lambda"));

			QScopedPointer<AbstractTestInstance> impl2(Env::create<AbstractTestInstance*>());
			QVERIFY(impl2);
			QCOMPARE(impl2->dummy(), QLatin1String("lambda"));
			QVERIFY(impl != impl2);
		}


		void mockArgs()
		{
			QScopedPointer<AbstractTestInstance> impl(Env::create<AbstractTestInstance*>(QString("flupp")));
			QVERIFY(impl);
			QCOMPARE(impl->dummy(), QLatin1String("flupp"));
			impl.reset();

			std::function<AbstractTestPodInstance*(QString)> funcString = [](const QString& pStr){
						class tmp
							: public AbstractTestPodInstance
						{
							const QString mDummy;

							public:
								tmp(const QString& pDummy)
									: mDummy(QStringLiteral("YEAH: ") + pDummy)
								{
								}


								virtual QString dummy() override
								{
									return mDummy;
								}


						};

						return new tmp(pStr);
					};

			std::function<AbstractTestPodInstance*()> funcDefault = [](){
						class tmp
							: public AbstractTestPodInstance
						{
							public:
								virtual QString dummy() override
								{
									return QStringLiteral("default");
								}


						};

						return new tmp();
					};

			Env::setCreator<AbstractTestPodInstance*, QString>(funcString);
			Env::setCreator<AbstractTestPodInstance*>(funcDefault);

			QScopedPointer<AbstractTestPodInstance> catcher(Env::create<AbstractTestPodInstance*>(QString("bla")));
			QVERIFY(catcher);
			QCOMPARE(catcher->dummy(), QLatin1String("YEAH: bla"));

			catcher.reset(Env::create<AbstractTestPodInstance*>());
			QVERIFY(catcher);
			QCOMPARE(catcher->dummy(), QLatin1String("default"));
		}


		void mockCopy()
		{
			class TestCopyCtor
			{
				private:
					int mData;

				public:
					TestCopyCtor(int pData)
						: mData(pData)
					{
					}


					int data()
					{
						return mData;
					}


			};

			auto obj = Env::create<TestCopyCtor>(666);
			QCOMPARE(obj.data(), 666);
			obj = Env::create<TestCopyCtor>(1904);
			QCOMPARE(obj.data(), 1904);

			std::function<TestCopyCtor(int)> func = [](int){
						return TestCopyCtor(1);
					};
			Env::setCreator<TestCopyCtor, int>(func);

			obj = Env::create<TestCopyCtor>(1982);
			QCOMPARE(obj.data(), 1);
		}


		void mockMove()
		{
			auto obj = Env::create<TestMoveCtorAssign>(QString("huhu"));
			QCOMPARE(obj.data(), QLatin1String("huhu"));

			std::function<TestMoveCtorAssign(QString)> func = [](const QString&){
						return TestMoveCtorAssign(QStringLiteral("mocked"));
					};
			Env::setCreator<TestMoveCtorAssign, QString>(func);

			auto mock = Env::create<TestMoveCtorAssign>(QString("huhu"));
			QCOMPARE(mock.data(), QLatin1String("mocked"));
			QCOMPARE(obj.data(), QLatin1String("huhu"));

			func = [](const QString&){
						return TestMoveCtorAssign(QStringLiteral("mocked2"));
					};
			Env::setCreator<TestMoveCtorAssign, QString>(func);
			mock = Env::create<TestMoveCtorAssign>(QString("huhu2"));
			QCOMPARE(mock.data(), QLatin1String("mocked2"));
			QCOMPARE(obj.data(), QLatin1String("huhu"));
		}


		void mockCounter()
		{
			struct TestTmp
			{
				int mData;
				TestTmp(int pData = 666)
					: mData(pData)
				{
				}


			};

			auto obj = Env::create<TestTmp>();
			QCOMPARE(obj.mData, 666);

			obj = Env::create<TestTmp>(1904);
			QCOMPARE(obj.mData, 1904);

			const std::function<TestTmp()> func = [](){
						return TestTmp(999);
					};
			QCOMPARE(Env::getCounter<TestTmp>(), -1);
			Env::setCreator<TestTmp>(func);
			QCOMPARE(Env::getCounter<TestTmp>(), 0);

			obj = Env::create<TestTmp>();
			QCOMPARE(obj.mData, 999);
			QCOMPARE(Env::getCounter<TestTmp>(), 1);

			obj = Env::create<TestTmp>(123);
			QCOMPARE(obj.mData, 123);
			QCOMPARE(Env::getCounter<TestTmp>(), 1);

			const std::function<TestTmp(int)> func2 = [](int){
						return TestTmp(777);
					};
			QCOMPARE((Env::getCounter<TestTmp, int>()), -1);

			Env::setCreator<TestTmp>(func2);
			QCOMPARE(Env::getCounter<TestTmp>(), 1);
			QCOMPARE((Env::getCounter<TestTmp, int>()), 0);

			obj = Env::create<TestTmp>();
			QCOMPARE(obj.mData, 999);
			QCOMPARE(Env::getCounter<TestTmp>(), 2);
			QCOMPARE((Env::getCounter<TestTmp, int>()), 0);

			obj = Env::create<TestTmp>(123);
			QCOMPARE(obj.mData, 777);
			QCOMPARE(Env::getCounter<TestTmp>(), 2);
			QCOMPARE((Env::getCounter<TestTmp, int>()), 1);

			Env::resetCounter();
			QCOMPARE(Env::getCounter<TestTmp>(), 0);
			QCOMPARE((Env::getCounter<TestTmp, int>()), 0);

			Env::clear();
			QCOMPARE(Env::getCounter<TestTmp>(), -1);
			QCOMPARE((Env::getCounter<TestTmp, int>()), -1);
		}


};

QTEST_GUILESS_MAIN(test_Env)
#include "test_Env.moc"
