#ifndef WEBSOCKETSERVERP_H
#define WEBSOCKETSERVERP_H

#include <QObject>
#include <QQueue>
#include <QString>
#include "websocket.h"

class QTcpServer;

class WebSocketServerImp: public QObject
{
	Q_OBJECT

public:
	WebSocketServerImp(QObject *parent = 0);
	virtual ~WebSocketServerImp();

	QString composeOpeningHandshake();
	QString composeBadRequest();

	WebSocket *nextPendingConnection();

	QList<WebSocketProtocol::Version> getSupportedVersions() const;
	QList<QString> getSupportedProtocols() const;
	QList<QString> getSupportedExtensions() const;

Q_SIGNALS:
	void newConnection();

private Q_SLOTS:
	void newConnection();
	void closeConnection();
	void handshakeReceived();

private:
	QTcpServer *m_pTcpServer;
	QQueue<WebSocket *> m_pendingConnections;

	void addPendingConnection(WebSocket *pWebSocket);
};

#endif // WEBSOCKETSERVERP_H
