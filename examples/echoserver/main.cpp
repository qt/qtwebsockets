#include <QCoreApplication>
#include "echoserver.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	EchoServer server(1234);

	Q_UNUSED(server);

	return a.exec();
}
