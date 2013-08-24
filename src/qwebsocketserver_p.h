/**
 * @file websocketserver_p.h
 * @author Kurt Pattyn (pattyn.kurt@gmail.com)
 * @brief Defines the private WebSocketServerPrivate class.
 */

#ifndef QWEBSOCKETSERVER_P_H
#define QWEBSOCKETSERVER_P_H

#include <QObject>
#include <QQueue>
#include <QString>
#include <QHostAddress>
#include "qwebsocket.h"

class QTcpServer;
class QWebSocketServer;

class QWebSocketServerPrivate : public QObject
{
	Q_OBJECT

public:
	explicit QWebSocketServerPrivate(const QString &serverName, QWebSocketServer * const pWebSocketServer, QObject *parent = 0);
	virtual ~QWebSocketServerPrivate();

	void close();
	QString errorString() const;
	bool hasPendingConnections() const;
	bool isListening() const;
	bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
	int maxPendingConnections() const;
	virtual QWebSocket *nextPendingConnection();
	QNetworkProxy proxy() const;
	QHostAddress serverAddress() const;
	QAbstractSocket::SocketError serverError() const;
	quint16 serverPort() const;
	void setMaxPendingConnections(int numConnections);
	void setProxy(const QNetworkProxy &networkProxy);
	bool setSocketDescriptor(int socketDescriptor);
	int socketDescriptor() const;
	bool waitForNewConnection(int msec = 0, bool *timedOut = 0);

	QList<QWebSocketProtocol::Version> supportedVersions() const;
	QList<QString> supportedProtocols() const;
	QList<QString> supportedExtensions() const;

Q_SIGNALS:
	void newConnection();

private Q_SLOTS:
	void onNewConnection();
	void onCloseConnection();
	void handshakeReceived();

private:
	QWebSocketServer * const q_ptr;

	QTcpServer *m_pTcpServer;
	QString m_serverName;
	QQueue<QWebSocket *> m_pendingConnections;

	void addPendingConnection(QWebSocket *pWebSocket);
};

#endif // QWEBSOCKETSERVER_P_H
