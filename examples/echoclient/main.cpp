#include <QCoreApplication>
#include "websocketclient.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	WebSocketClient client;

	Q_UNUSED(client);

	return a.exec();
}
