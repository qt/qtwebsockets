/**
 * @file websocketserver.h
 * @author Kurt Pattyn (pattyn.kurt@gmail.com)
 * @brief Defines the WebSocketServer class.
 */

#ifndef QWEBSOCKETSERVER_H
#define QWEBSOCKETSERVER_H

#include <QObject>
#include <QString>
#include <QHostAddress>
#include "qwebsocketsglobal.h"
#include "qwebsocketprotocol.h"

QT_BEGIN_NAMESPACE

class QWebSocketServerPrivate;
class QWebSocket;

class Q_WEBSOCKETS_EXPORT QWebSocketServer : public QObject
{
	Q_OBJECT

public:
	explicit QWebSocketServer(const QString &serverName, QObject *parent = 0);
	virtual ~QWebSocketServer();

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
#ifndef QT_NO_NETWORKPROXY
	void setProxy(const QNetworkProxy &networkProxy);
	QNetworkProxy proxy() const;
#endif

	QList<QWebSocketProtocol::Version> supportedVersions() const;
	QList<QString> supportedProtocols() const;
	QList<QString> supportedExtensions() const;

protected:
	virtual bool isOriginAllowed(const QString &origin) const;

Q_SIGNALS:
	void newConnection();

private:
	Q_DISABLE_COPY(QWebSocketServer)
	QWebSocketServerPrivate * const d_ptr;
	friend class QWebSocketServerPrivate;
};

QT_END_NAMESPACE

#endif // QWEBSOCKETSERVER_H
