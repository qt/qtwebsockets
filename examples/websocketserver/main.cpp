#include <QCoreApplication>
#include "helloworldserver.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	HelloWorldServer server(1234);

	Q_UNUSED(server);

	return a.exec();
}
