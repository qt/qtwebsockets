#include "websocketclient.h"
#include <QDebug>

WebSocketClient::WebSocketClient(QObject *parent) :
	QObject(parent),
	m_webSocket()
{
	connect(&m_webSocket, SIGNAL(connected()), this, SLOT(onConnected()));
	m_webSocket.open(QUrl("ws://localhost:1234"));
}

void WebSocketClient::onConnected()
{
	qDebug() << "Websocket connected";
	connect(&m_webSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
	m_webSocket.send("Hello, world!");
}

void WebSocketClient::onTextMessageReceived(QString message)
{
	qDebug() << "Message received:" << message;
}
