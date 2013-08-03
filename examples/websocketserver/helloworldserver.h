#ifndef HELLOWORLDSERVER_H
#define HELLOWORLDSERVER_H

#include <QObject>
#include <QList>

class WebSocketServer;
class WebSocket;

class HelloWorldServer : public QObject
{
	Q_OBJECT
public:
	explicit HelloWorldServer(quint16 port, QObject *parent = 0);

Q_SIGNALS:

private Q_SLOTS:
	void onNewConnection();
	void processMessage(QString message, bool isLastFrame);
	void socketDisconnected();

private:
	WebSocketServer *m_pWebSocketServer;
	QList<WebSocket *> m_clients;
};

#endif // HELLOWORLDSERVER_H
