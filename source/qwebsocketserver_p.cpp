#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkProxy>
#include "qwebsocketserver.h"
#include "qwebsocketserver_p.h"
#include "qwebsocketprotocol.h"
#include "handshakerequest_p.h"
#include "handshakeresponse_p.h"
#include "qwebsocket.h"

QWebSocketServerPrivate::QWebSocketServerPrivate(const QString &serverName, QWebSocketServer * const pWebSocketServer, QObject *parent) :
	QObject(parent),
	q_ptr(pWebSocketServer),
	m_pTcpServer(0),
	m_serverName(serverName),
	m_pendingConnections()
{
	Q_ASSERT(pWebSocketServer != 0);
	m_pTcpServer = new QTcpServer(this);
	connect(m_pTcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
}

QWebSocketServerPrivate::~QWebSocketServerPrivate()
{
	while (!m_pendingConnections.isEmpty())
	{
		QWebSocket *pWebSocket = m_pendingConnections.dequeue();
		pWebSocket->close(QWebSocketProtocol::CC_GOING_AWAY, "Server closed.");
		pWebSocket->deleteLater();
	}
	m_pTcpServer->deleteLater();
}

void QWebSocketServerPrivate::close()
{
	m_pTcpServer->close();
}

QString QWebSocketServerPrivate::errorString() const
{
	return m_pTcpServer->errorString();
}

bool QWebSocketServerPrivate::hasPendingConnections() const
{
	return !m_pendingConnections.isEmpty();
}

bool QWebSocketServerPrivate::isListening() const
{
	return m_pTcpServer->isListening();
}

bool QWebSocketServerPrivate::listen(const QHostAddress &address, quint16 port)
{
	return m_pTcpServer->listen(address, port);
}

int QWebSocketServerPrivate::maxPendingConnections() const
{
	return m_pTcpServer->maxPendingConnections();
}

void QWebSocketServerPrivate::addPendingConnection(QWebSocket *pWebSocket)
{
	if (m_pendingConnections.size() < maxPendingConnections())
	{
		m_pendingConnections.enqueue(pWebSocket);
	}
}

QWebSocket *QWebSocketServerPrivate::nextPendingConnection()
{
	QWebSocket *pWebSocket = 0;
	if (!m_pendingConnections.isEmpty())
	{
		pWebSocket = m_pendingConnections.dequeue();
	}
	return pWebSocket;
}

QNetworkProxy QWebSocketServerPrivate::proxy() const
{
	return m_pTcpServer->proxy();
}

QHostAddress QWebSocketServerPrivate::serverAddress() const
{
	return m_pTcpServer->serverAddress();
}

QAbstractSocket::SocketError QWebSocketServerPrivate::serverError() const
{
	return m_pTcpServer->serverError();
}

quint16 QWebSocketServerPrivate::serverPort() const
{
	return m_pTcpServer->serverPort();
}

void QWebSocketServerPrivate::setMaxPendingConnections(int numConnections)
{
	m_pTcpServer->setMaxPendingConnections(numConnections);
}

void QWebSocketServerPrivate::setProxy(const QNetworkProxy &networkProxy)
{
	m_pTcpServer->setProxy(networkProxy);
}

bool QWebSocketServerPrivate::setSocketDescriptor(int socketDescriptor)
{
	return m_pTcpServer->setSocketDescriptor(socketDescriptor);
}

int QWebSocketServerPrivate::socketDescriptor() const
{
	return m_pTcpServer->socketDescriptor();
}

bool QWebSocketServerPrivate::waitForNewConnection(int msec, bool *timedOut)
{
	return m_pTcpServer->waitForNewConnection(msec, timedOut);
}

QList<QWebSocketProtocol::Version> QWebSocketServerPrivate::supportedVersions() const
{
	QList<QWebSocketProtocol::Version> supportedVersions;
	supportedVersions << QWebSocketProtocol::currentVersion();	//we only support V13
	return supportedVersions;
}

QList<QString> QWebSocketServerPrivate::supportedProtocols() const
{
	QList<QString> supportedProtocols;
	return supportedProtocols;	//no protocols are currently supported
}

QList<QString> QWebSocketServerPrivate::supportedExtensions() const
{
	QList<QString> supportedExtensions;
	return supportedExtensions;	//no extensions are currently supported
}

void QWebSocketServerPrivate::onNewConnection()
{
	QTcpSocket *pTcpSocket = m_pTcpServer->nextPendingConnection();
	connect(pTcpSocket, SIGNAL(readyRead()), this, SLOT(handshakeReceived()));
}

void QWebSocketServerPrivate::onCloseConnection()
{
	QTcpSocket *pTcpSocket = qobject_cast<QTcpSocket*>(sender());
	if (pTcpSocket != 0)
	{
		pTcpSocket->close();
	}
}

void QWebSocketServerPrivate::handshakeReceived()
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
								   m_serverName,
								   q_ptr->isOriginAllowed(request.getOrigin()),
								   supportedVersions(),
								   supportedProtocols(),
								   supportedExtensions());
		disconnect(pTcpSocket, SIGNAL(readyRead()), this, SLOT(handshakeReceived()));

		if (response.isValid())
		{
			QTextStream httpStream(pTcpSocket);
			httpStream << response;
			httpStream.flush();

			if (response.canUpgrade())
			{
				QWebSocket *pWebSocket = QWebSocket::upgradeFrom(pTcpSocket, request, response);
				if (pWebSocket)
				{
					pWebSocket->setParent(this);
					addPendingConnection(pWebSocket);
					Q_EMIT q_ptr->newConnection();
					success = true;
				}
				else
				{
					qDebug() << "QWebSocketServerPrivate::handshakeReceived: Upgrading to WebSocket failed.";
				}
			}
			else
			{
				qDebug() << "QWebSocketServerPrivate::handshakeReceived: Cannot upgrade to websocket.";
			}
		}
		else
		{
			qDebug() << "QWebSocketServerPrivate::handshakeReceived: Invalid response. This should not happen!!!";
		}
		if (!success)
		{
			qDebug() << "QWebSocketServerPrivate::handshakeReceived: Closing socket because of invalid or unsupported request";
			pTcpSocket->close();
		}
	}
	else
	{
		qDebug() << "WebSocketServerPrivate::handshakeReceived: Sender socket is NULL. This should not happen!!!";
	}
}
