#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include "websocket.h"

class WebSocketClient : public QObject
{
	Q_OBJECT
public:
	explicit WebSocketClient(QObject *parent = 0);

Q_SIGNALS:

public Q_SLOTS:

private Q_SLOTS:
	void onConnected();
	void onTextMessageReceived(QString message);

private:
	WebSocket m_webSocket;
};

#endif // WEBSOCKETCLIENT_H
