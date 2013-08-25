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

	bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
	void close();

	bool isListening() const;

	void setMaxPendingConnections(int numConnections);
	int maxPendingConnections() const;

	quint16 serverPort() const;
	QHostAddress serverAddress() const;

	bool setSocketDescriptor(int socketDescriptor);
	int socketDescriptor() const;

	bool waitForNewConnection(int msec = 0, bool *timedOut = 0);
	bool hasPendingConnections() const;
	virtual QWebSocket *nextPendingConnection();

	QAbstractSocket::SocketError serverError() const;
	QString errorString() const;

	void pauseAccepting();
	void resumeAccepting();

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
	void acceptError(QAbstractSocket::SocketError socketError);
	void newConnection();

private:
	Q_DISABLE_COPY(QWebSocketServer)
	QWebSocketServerPrivate * const d_ptr;
	friend class QWebSocketServerPrivate;
};

QT_END_NAMESPACE

#endif // QWEBSOCKETSERVER_H
