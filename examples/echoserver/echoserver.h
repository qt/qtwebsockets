#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include <QObject>
#include <QList>
#include <QByteArray>

class WebSocketServer;
class WebSocket;

class EchoServer : public QObject
{
	Q_OBJECT
public:
	explicit EchoServer(quint16 port, QObject *parent = 0);

Q_SIGNALS:

private Q_SLOTS:
	void onNewConnection();
	void processMessage(QString message);
	void processBinaryMessage(QByteArray message);
	void socketDisconnected();

private:
	WebSocketServer *m_pWebSocketServer;
	QList<WebSocket *> m_clients;
};

#endif //ECHOSERVER_H
