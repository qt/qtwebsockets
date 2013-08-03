#include "websocketserver_p.h"
#include <QTcpServer>
#include <QTextStream>
#include <QUrl>
#include <QTcpSocket>
#include <QDateTime>
#include "websocketprotocol.h"
#include "handshakerequest.h"
#include "handshakeresponse.h"
#include "websocket.h"

WebSocketServerImp::WebSocketServerImp(QObject *parent) :
	QObject(parent),
	m_pTcpServer(0),
	m_pendingConnections()
{
	m_pTcpServer = new QTcpServer(this);
	connect(m_pTcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

WebSocketServerImp::~WebSocketServerImp()
{
	m_pTcpServer->deleteLater();
}

void WebSocketServerImp::addPendingConnection(WebSocket *pWebSocket)
{
	m_pendingConnections.enqueue(pWebSocket);
}

WebSocket *WebSocketServerImp::nextPendingConnection()
{
	WebSocket *pWebSocket = 0;
	if (!m_pendingConnections.isEmpty())
	{
		pWebSocket = m_pendingConnections.dequeue();
	}
	return pWebSocket;
}

QList<WebSocketProtocol::Version> WebSocketServerImp::getSupportedVersions() const
{
	//we only support V13 for now
	QList<WebSocketProtocol::Version> supportedVersions;
	supportedVersions << WebSocketProtocol::V_13;
	return supportedVersions;
}

QList<QString> WebSocketServerImp::getSupportedProtocols() const
{
	QList<QString> supportedProtocols;
	return supportedProtocols;	//no protocols are currently supported
}

QList<QString> WebSocketServerImp::getSupportedExtensions() const
{
	QList<QString> supportedExtensions;
	return supportedExtensions;	//no extensions are currently supported
}

void WebSocketServerImp::newConnection()
{
	QTcpSocket *tcpSocket = m_pTcpServer->nextPendingConnection();
	connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(handshakeReceived()));
}

void WebSocketServerImp::closeConnection()
{
	QTcpSocket *tcpSocket = qobject_cast<QTcpSocket*>(sender());
	if (tcpSocket != 0)
	{
		tcpSocket->close();
	}
}

void WebSocketServerImp::handshakeReceived()
{
	QTcpSocket *pTcpSocket = qobject_cast<QTcpSocket*>(sender());
	if (pTcpSocket != 0)
	{
		bool success = false;
		bool isSecure = false;
		HandshakeRequest request(pTcpSocket->peerPort(), isSecure);
		QTextStream textStream(pTcpSocket);
		textStream >> request;

		HandshakeResponse response(request,
								   getSupportedVersions(),
								   getSupportedProtocols(),
								   getSupportedExtensions());
		disconnect(pTcpSocket, SIGNAL(readyRead()), this, SLOT(handshakeReceived()));

		if (response.isValid())
		{
			QTextStream httpStream(pTcpSocket);
			httpStream << response;

			if (response.canUpgrade())
			{
				WebSocket *pWebSocket = WebSocket::upgradeFrom(pTcpSocket, request, response);
				if (pWebSocket)
				{
					pWebSocket->setParent(this);
					addPendingConnection(pWebSocket);
					Q_EMIT newConnection();
					success = true;
				}
				else
				{
					qDebug() << "WebSocketServerImp::dataReceived: Upgrading to WebSocket failed.";
				}
			}
			else
			{
				qDebug() << "WebSocketServerImp::dataReceived: Cannot upgrade to websocket.";
			}
		}
		else
		{
			qDebug() << "WebSocketServerImp::dataReceived: Invalid response. This should not happen!!!";
		}
		if (!success)
		{
			qDebug() << "WeBsocketServerImp::dataReceived: Closing socket because of invalid or unsupported request";
			pTcpSocket->close();
		}
	}
	else
	{
		qDebug() << "WebSocketServerImp::dataReceived: Sender socket is NULL. This should not happen!!!";
	}
}
