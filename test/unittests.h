#ifndef UNITTESTT_H
#define UNITTESTT_H

#include <QTest>
#include <QObject>
#include <QList>
#include <QString>
#include <QSharedPointer>
#include <QCoreApplication>

namespace AutoTest
{

typedef QList<QObject*> TestList;

inline TestList& testList()
{
	static TestList list;
	return list;
}

inline bool findObject(QObject* object)
{
	TestList& list = testList();
	if (list.contains(object))
	{
		return true;
	}
	Q_FOREACH (QObject* test, list)
	{
		if (test->objectName() == object->objectName())
		{
			return true;
		}
	}
	return false;
}

inline void addTest(QObject* object)
{
	TestList& list = testList();
	if (!findObject(object))
	{
		list.append(object);
	}
}

inline int run(int argc, char *argv[])
{
	int ret = 0;

	Q_FOREACH (QObject* test, testList())
	{
		ret += QTest::qExec(test, argc, argv);
	}
	testList().clear();
	return ret;
}

} // end namespace

template <class T>
class Test
{
public:
	QSharedPointer<T> child;

	Test(const QString& name) : child(new T)
	{
	child->setObjectName(name);
	AutoTest::addTest(child.data());
	}
};

#define DECLARE_TEST(className) static Test<className> t(#className);

#define TEST_MAIN \
int main(int argc, char *argv[]) \
{ \
	QCoreApplication app(argc, argv); \
	int ret = AutoTest::run(argc, argv); \
	return ret; \
}
//return app.exec();

#endif
