#include "echoclient.h"
#include <QDebug>

EchoClient::EchoClient(const QUrl &url, QObject *parent) :
	QObject(parent),
	m_webSocket()
{
	connect(&m_webSocket, SIGNAL(connected()), this, SLOT(onConnected()));
	m_webSocket.open(QUrl(url));
}

void EchoClient::onConnected()
{
	qDebug() << "Websocket connected";
	connect(&m_webSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
	m_webSocket.send("Hello, world!");
}

void EchoClient::onTextMessageReceived(QString message)
{
	qDebug() << "Message received:" << message;
}
