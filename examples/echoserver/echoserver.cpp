#include "echoserver.h"
#include "websocketserver.h"
#include "websocket.h"
#include <QDebug>

//! [constructor]
EchoServer::EchoServer(quint16 port, QObject *parent) :
	QObject(parent),
	m_pWebSocketServer(0),
	m_clients()
{
	m_pWebSocketServer = new WebSocketServer(this);
	if (m_pWebSocketServer->listen(QHostAddress::Any, port))
	{
		qDebug() << "Echoserver listening on port" << port;
		connect(m_pWebSocketServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
	}
}
//! [constructor]

//! [onNewConnection]
void EchoServer::onNewConnection()
{
	WebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

	connect(pSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(processMessage(QString)));
	connect(pSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(processBinaryMessage(QByteArray)));
	connect(pSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	//connect(pSocket, SIGNAL(pong(quint64)), this, SLOT(processPong(quint64)));

	m_clients << pSocket;
}
//! [onNewConnection]

//! [processMessage]
void EchoServer::processMessage(QString message)
{
	WebSocket *pClient = qobject_cast<WebSocket *>(sender());
	if (pClient != 0)
	{
		pClient->send(message);
	}
}
//! [processMessage]

//! [processBinaryMessage]
void EchoServer::processBinaryMessage(QByteArray message)
{
	WebSocket *pClient = qobject_cast<WebSocket *>(sender());
	if (pClient != 0)
	{
		pClient->send(message);
	}
}
//! [processBinaryMessage]

//! [socketDisconnected]
void EchoServer::socketDisconnected()
{
	WebSocket *pClient = qobject_cast<WebSocket *>(sender());
	if (pClient != 0)
	{
		m_clients.removeAll(pClient);
		pClient->deleteLater();
	}
}
//! [socketDisconnected]
