#ifndef ECHOCLIENT_H
#define ECHOCLIENT_H

#include <QObject>
#include "websocket.h"

class EchoClient : public QObject
{
	Q_OBJECT
public:
	explicit EchoClient(const QUrl &url, QObject *parent = 0);

Q_SIGNALS:

public Q_SLOTS:

private Q_SLOTS:
	void onConnected();
	void onTextMessageReceived(QString message);

private:
	WebSocket m_webSocket;
};

#endif // ECHOCLIENT_H
