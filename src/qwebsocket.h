/**
 * @file websocket.h
 * @brief Defines the WebSocket class.
 *
 * \note Currently, only V13 (RFC6455) is supported.
 * \note Both text and binary websockets are supported.
 * \note The secure version (wss) is currently not implemented.
 * @author Kurt Pattyn (pattyn.kurt@gmail.com)
 */

#ifndef QWEBSOCKET_H
#define QWEBSOCKET_H

#include <QUrl>
#include <QAbstractSocket>
#include <QHostAddress>
#ifndef QT_NO_NETWORKPROXY
#include <QNetworkProxy>
#endif
#include <QTime>
#include "qwebsocketsglobal.h"
#include "qwebsocketprotocol.h"

QT_BEGIN_NAMESPACE

class QTcpSocket;
class QWebSocketPrivate;

class Q_WEBSOCKETS_EXPORT QWebSocket:public QObject
{
	Q_OBJECT

public:
	explicit QWebSocket(QString origin = QString(), QWebSocketProtocol::Version version = QWebSocketProtocol::V_LATEST, QObject *parent = 0);
	virtual ~QWebSocket();

	void abort();
	QAbstractSocket::SocketError error() const;
	QString errorString() const;
	bool flush();
	bool isValid();
	QHostAddress localAddress() const;
	quint16 localPort() const;
	QHostAddress peerAddress() const;
	QString peerName() const;
	quint16 peerPort() const;
#ifndef QT_NO_NETWORKPROXY
	QNetworkProxy proxy() const;
	void setProxy(const QNetworkProxy &networkProxy);
#endif
	qint64 readBufferSize() const;
	void setReadBufferSize(qint64 size);
	void setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value);
	QVariant socketOption(QAbstractSocket::SocketOption option);
	QAbstractSocket::SocketState state() const;

	bool waitForConnected(int msecs = 30000);
	bool waitForDisconnected(int msecs = 30000);

	QWebSocketProtocol::Version version();
	QString resourceName();
	QUrl requestUrl();
	QString origin();
	QString protocol();
	QString extension();

	qint64 write(const char *message);		//send data as text
	qint64 write(const char *message, qint64 maxSize);		//send data as text
	qint64 write(const QString &message);	//send data as text
	qint64 write(const QByteArray &data);	//send data as binary

public Q_SLOTS:
	virtual void close(QWebSocketProtocol::CloseCode closeCode = QWebSocketProtocol::CC_NORMAL, QString reason = QString());
	virtual void open(const QUrl &url, bool mask = true);
	void ping();

Q_SIGNALS:
	void aboutToClose();
	void connected();
	void disconnected();
	void stateChanged(QAbstractSocket::SocketState state);
#ifndef QT_NO_NETWORKPROXY
	void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *pAuthenticator);
#endif
	void readChannelFinished();
	void textFrameReceived(QString frame, bool isLastFrame);
	void binaryFrameReceived(QByteArray frame, bool isLastFrame);
	void textMessageReceived(QString message);
	void binaryMessageReceived(QByteArray message);
	void error(QAbstractSocket::SocketError error);
	void pong(quint64 elapsedTime);

private:
	Q_DISABLE_COPY(QWebSocket)
	QWebSocket(QTcpSocket *pTcpSocket, QWebSocketProtocol::Version version, QObject *parent = 0);
	QWebSocketPrivate * const d_ptr;

	friend class QWebSocketPrivate;
};

QT_END_NAMESPACE

#endif // QWEBSOCKET_H
