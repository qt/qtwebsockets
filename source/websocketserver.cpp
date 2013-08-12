#include "websocketserver.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkProxy>
#include "websocketprotocol.h"
#include "handshakerequest.h"
#include "handshakeresponse.h"
#include "websocket.h"

WebSocketServer::WebSocketServer(QObject *parent) :
	QObject(parent),
	m_pTcpServer(0),
	m_pendingConnections()
{
	m_pTcpServer = new QTcpServer(this);
	connect(m_pTcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
}

WebSocketServer::~WebSocketServer()
{
	m_pTcpServer->deleteLater();
}

void WebSocketServer::close()
{
	m_pTcpServer->close();
}

QString WebSocketServer::errorString() const
{
	return m_pTcpServer->errorString();
}

bool WebSocketServer::hasPendingConnections() const
{
	return !m_pendingConnections.isEmpty();
}

bool WebSocketServer::isListening() const
{
	return m_pTcpServer->isListening();
}

bool WebSocketServer::listen(const QHostAddress &address, quint16 port)
{
	return m_pTcpServer->listen(address, port);
}

int WebSocketServer::maxPendingConnections() const
{
	return m_pTcpServer->maxPendingConnections();
}

void WebSocketServer::addPendingConnection(WebSocket *pWebSocket)
{
	if (m_pendingConnections.size() < maxPendingConnections())
	{
		m_pendingConnections.enqueue(pWebSocket);
	}
}

WebSocket *WebSocketServer::nextPendingConnection()
{
	WebSocket *pWebSocket = 0;
	if (!m_pendingConnections.isEmpty())
	{
		pWebSocket = m_pendingConnections.dequeue();
	}
	return pWebSocket;
}

QNetworkProxy WebSocketServer::proxy() const
{
	return m_pTcpServer->proxy();
}

QHostAddress WebSocketServer::serverAddress() const
{
	return m_pTcpServer->serverAddress();
}

QAbstractSocket::SocketError WebSocketServer::serverError() const
{
	return m_pTcpServer->serverError();
}

quint16 WebSocketServer::serverPort() const
{
	return m_pTcpServer->serverPort();
}

void WebSocketServer::setMaxPendingConnections(int numConnections)
{
	m_pTcpServer->setMaxPendingConnections(numConnections);
}

void WebSocketServer::setProxy(const QNetworkProxy &networkProxy)
{
	m_pTcpServer->setProxy(networkProxy);
}

bool WebSocketServer::setSocketDescriptor(int socketDescriptor)
{
	return m_pTcpServer->setSocketDescriptor(socketDescriptor);
}

int WebSocketServer::socketDescriptor() const
{
	return m_pTcpServer->socketDescriptor();
}

bool WebSocketServer::waitForNewConnection(int msec, bool *timedOut)
{
	return m_pTcpServer->waitForNewConnection(msec, timedOut);
}

QList<WebSocketProtocol::Version> WebSocketServer::getSupportedVersions() const
{
	QList<WebSocketProtocol::Version> supportedVersions;
	supportedVersions << WebSocketProtocol::getCurrentVersion();	//we only support V13
	return supportedVersions;
}

QList<QString> WebSocketServer::getSupportedProtocols() const
{
	QList<QString> supportedProtocols;
	return supportedProtocols;	//no protocols are currently supported
}

QList<QString> WebSocketServer::getSupportedExtensions() const
{
	QList<QString> supportedExtensions;
	return supportedExtensions;	//no extensions are currently supported
}

void WebSocketServer::onNewConnection()
{
	QTcpSocket *pTcpSocket = m_pTcpServer->nextPendingConnection();
	connect(pTcpSocket, SIGNAL(readyRead()), this, SLOT(handshakeReceived()));
}

void WebSocketServer::onCloseConnection()
{
	QTcpSocket *pTcpSocket = qobject_cast<QTcpSocket*>(sender());
	if (pTcpSocket != 0)
	{
		pTcpSocket->close();
	}
}

void WebSocketServer::handshakeReceived()
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
			httpStream.flush();

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
					qDebug() << "WebSocketServer::dataReceived: Upgrading to WebSocket failed.";
				}
			}
			else
			{
				qDebug() << "WebSocketServer::dataReceived: Cannot upgrade to websocket.";
			}
		}
		else
		{
			qDebug() << "WebSocketServer::dataReceived: Invalid response. This should not happen!!!";
		}
		if (!success)
		{
			qDebug() << "WebSocketServer::dataReceived: Closing socket because of invalid or unsupported request";
			pTcpSocket->close();
		}
	}
	else
	{
		qDebug() << "WebSocketServerImp::dataReceived: Sender socket is NULL. This should not happen!!!";
	}
}
