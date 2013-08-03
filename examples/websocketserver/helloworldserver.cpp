#include "helloworldserver.h"
#include "websocketserver.h"
#include "websocket.h"
#include <QDebug>

HelloWorldServer::HelloWorldServer(quint16 port, QObject *parent) :
	QObject(parent),
	m_pWebSocketServer(0),
	m_clients()
{
	m_pWebSocketServer = new WebSocketServer(this);
	if (m_pWebSocketServer->listen(QHostAddress::Any, port))
	{
		qDebug() << "HelloWorld Server listening on port" << port;
		connect(m_pWebSocketServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
	}
}

void HelloWorldServer::onNewConnection()
{
	qDebug() << "Client connected.";
	WebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

	connect(pSocket, SIGNAL(textFrameReceived(QString,bool)), this, SLOT(processMessage(QString, bool)));
	//connect(pSocket, SIGNAL(binaryFrameReceived(QByteArray,bool)), this, SLOT(processBinaryMessage(QByteArray)));
	connect(pSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	//connect(pSocket, SIGNAL(pong(quint64)), this, SLOT(processPong(quint64)));

	m_clients << pSocket;
}

void HelloWorldServer::processMessage(QString message, bool isLastFrame)
{
	Q_UNUSED(isLastFrame);
	WebSocket *pClient = qobject_cast<WebSocket *>(sender());
	if (pClient != 0)
	{
		QString answer;
		for (int i = 0; i < message.length(); ++i)
		{
			answer.push_front(message[i]);
		}
		pClient->send(answer);
	}
}

void HelloWorldServer::socketDisconnected()
{
	WebSocket *pClient = qobject_cast<WebSocket *>(sender());
	if (pClient != 0)
	{
		qDebug() << "Client disconnected";
		m_clients.removeAll(pClient);
		pClient->deleteLater();
	}
}
