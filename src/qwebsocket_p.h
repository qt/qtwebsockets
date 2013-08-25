/**
 * @file websocket.h
 * @brief Defines the WebSocket class.
 *
 * \note Currently, only V13 (RFC6455) is supported.
 * \note Both text and binary websockets are supported.
 * \note The secure version (wss) is currently not implemented.
 * @author Kurt Pattyn (pattyn.kurt@gmail.com)
 */

#ifndef QWEBSOCKET_P_H
#define QWEBSOCKET_P_H
//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QUrl>
#include <QAbstractSocket>
#include <QHostAddress>
#ifndef QT_NO_NETWORKPROXY
#include <QNetworkProxy>
#endif
#include <QTime>
#include "qwebsocketsglobal.h"
#include "qwebsocketprotocol.h"
#include "dataprocessor_p.h"

QT_BEGIN_NAMESPACE

class HandshakeRequest;
class HandshakeResponse;
class QTcpSocket;
class QWebSocket;

class QWebSocketPrivate:public QObject
{
	Q_OBJECT

public:
	explicit QWebSocketPrivate(QString origin,
							   QWebSocketProtocol::Version version,
							   QWebSocket * const pWebSocket,
							   QObject *parent = 0);
	virtual ~QWebSocketPrivate();

	void abort();
	QAbstractSocket::SocketError error() const;
	QString errorString() const;
	bool flush();
	bool isValid();
	QHostAddress localAddress() const;
	quint16 localPort() const;
	QAbstractSocket::PauseModes pauseMode() const;
	QHostAddress peerAddress() const;
	QString peerName() const;
	quint16 peerPort() const;
#ifndef QT_NO_NETWORKPROXY
	QNetworkProxy proxy() const;
	void setProxy(const QNetworkProxy &networkProxy);
#endif
	qint64 readBufferSize() const;
	void resume();
	void setPauseMode(QAbstractSocket::PauseModes pauseMode);
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

private Q_SLOTS:
	void processData();
	void processControlFrame(QWebSocketProtocol::OpCode opCode, QByteArray frame);
	void processHandshake(QTcpSocket *pSocket);
	void processStateChanged(QAbstractSocket::SocketState socketState);

private:
	Q_DISABLE_COPY(QWebSocketPrivate)

	QWebSocket * const q_ptr;

	QWebSocketPrivate(QTcpSocket *pTcpSocket, QWebSocketProtocol::Version version, QWebSocket *pWebSocket, QObject *parent = 0);
	void setVersion(QWebSocketProtocol::Version version);
	void setResourceName(QString resourceName);
	void setRequestUrl(QUrl requestUrl);
	void setOrigin(QString origin);
	void setProtocol(QString protocol);
	void setExtension(QString extension);
	void enableMasking(bool enable);
	void setSocketState(QAbstractSocket::SocketState state);
	void setErrorString(QString errorString);

	qint64 doWriteData(const QByteArray &data, bool isBinary);
	qint64 doWriteFrames(const QByteArray &data, bool isBinary);

	void makeConnections(const QTcpSocket *pTcpSocket);
	void releaseConnections(const QTcpSocket *pTcpSocket);

	QByteArray getFrameHeader(QWebSocketProtocol::OpCode opCode, quint64 payloadLength, quint32 maskingKey, bool lastFrame) const;
	QString calculateAcceptKey(const QString &key) const;
	QString createHandShakeRequest(QString resourceName,
								   QString host,
								   QString origin,
								   QString extensions,
								   QString protocols,
								   QByteArray key);

	static QWebSocket *upgradeFrom(QTcpSocket *tcpSocket,
								   const HandshakeRequest &request,
								   const HandshakeResponse &response,
								   QObject *parent = 0);

	quint32 generateMaskingKey() const;
	QByteArray generateKey() const;
	quint32 generateRandomNumber() const;
	qint64 writeFrames(const QList<QByteArray> &frames);
	qint64 writeFrame(const QByteArray &frame);

	QTcpSocket *m_pSocket;
	QString m_errorString;
	QWebSocketProtocol::Version m_version;
	QUrl m_resource;
	QString m_resourceName;
	QUrl m_requestUrl;
	QString m_origin;
	QString m_protocol;
	QString m_extension;
	QAbstractSocket::SocketState m_socketState;

	QByteArray m_key;	//identification key used in handshake requests

	bool m_mustMask;	//a server must not mask the frames it sends

	bool m_isClosingHandshakeSent;
	bool m_isClosingHandshakeReceived;

	QTime m_pingTimer;

	DataProcessor m_dataProcessor;


	friend class QWebSocketServerPrivate;
	friend class QWebSocket;
};

QT_END_NAMESPACE

#endif // QWEBSOCKET_H
