#include <QCoreApplication>
#include "echoclient.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	EchoClient client(QUrl("ws://localhost:1234"));

	Q_UNUSED(client);

	return a.exec();
}
