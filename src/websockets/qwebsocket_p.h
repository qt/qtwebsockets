/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

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

#include <QtCore/QUrl>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#ifndef QT_NO_NETWORKPROXY
#include <QtNetwork/QNetworkProxy>
#endif
#ifndef QT_NO_SSL
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslError>
#include <QtNetwork/QSslSocket>
#endif
#include <QtCore/QTime>

#include "qwebsocketprotocol.h"
#include "qwebsocketdataprocessor_p.h"

QT_BEGIN_NAMESPACE

class QWebSocketHandshakeRequest;
class QWebSocketHandshakeResponse;
class QTcpSocket;
class QWebSocket;

struct QWebSocketConfiguration
{
public:
    QWebSocketConfiguration();

public:
#ifndef QT_NO_SSL
    QSslConfiguration m_sslConfiguration;
    QList<QSslError> m_ignoredSslErrors;
    bool m_ignoreSslErrors;
#endif
#ifndef QT_NONETWORKPROXY
    QNetworkProxy m_proxy;
#endif
    QTcpSocket *m_pSocket;
};

class QWebSocketPrivate : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QWebSocketPrivate)
    Q_DECLARE_PUBLIC(QWebSocket)

public:
    explicit QWebSocketPrivate(const QString &origin,
                               QWebSocketProtocol::Version version,
                               QWebSocket * const pWebSocket,
                               QObject *parent = Q_NULLPTR);
    virtual ~QWebSocketPrivate();

    void abort();
    QAbstractSocket::SocketError error() const;
    QString errorString() const;
    bool flush();
    bool isValid() const;
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

    bool waitForConnected(int msecs);
    bool waitForDisconnected(int msecs);

    QWebSocketProtocol::Version version() const;
    QString resourceName() const;
    QUrl requestUrl() const;
    QString origin() const;
    QString protocol() const;
    QString extension() const;
    QWebSocketProtocol::CloseCode closeCode() const;
    QString closeReason() const;

    qint64 write(const char *message);		//send data as text
    qint64 write(const char *message, qint64 maxSize);		//send data as text
    qint64 write(const QString &message);	//send data as text
    qint64 write(const QByteArray &data);	//send data as binary

#ifndef QT_NO_SSL
    void ignoreSslErrors(const QList<QSslError> &errors);
    void setSslConfiguration(const QSslConfiguration &sslConfiguration);
    QSslConfiguration sslConfiguration() const;
#endif

public Q_SLOTS:
    void close(QWebSocketProtocol::CloseCode closeCode, QString reason);
    void open(const QUrl &url, bool mask);
    void ping(const QByteArray &payload);

#ifndef QT_NO_SSL
    void ignoreSslErrors();
#endif

private Q_SLOTS:
    void processData();
    void processPing(QByteArray data);
    void processPong(QByteArray data);
    void processClose(QWebSocketProtocol::CloseCode closeCode, QString closeReason);
    void processHandshake(QTcpSocket *pSocket);
    void processStateChanged(QAbstractSocket::SocketState socketState);

private:
    QWebSocket * const q_ptr;

    QWebSocketPrivate(QTcpSocket *pTcpSocket, QWebSocketProtocol::Version version, QWebSocket *pWebSocket, QObject *parent = Q_NULLPTR);
    void setVersion(QWebSocketProtocol::Version version);
    void setResourceName(const QString &resourceName);
    void setRequestUrl(const QUrl &requestUrl);
    void setOrigin(const QString &origin);
    void setProtocol(const QString &protocol);
    void setExtension(const QString &extension);
    void enableMasking(bool enable);
    void setSocketState(QAbstractSocket::SocketState state);
    void setErrorString(const QString &errorString);

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
                                   const QWebSocketHandshakeRequest &request,
                                   const QWebSocketHandshakeResponse &response,
                                   QObject *parent = Q_NULLPTR);

    quint32 generateMaskingKey() const;
    QByteArray generateKey() const;
    quint32 generateRandomNumber() const;
    qint64 writeFrames(const QList<QByteArray> &frames);
    qint64 writeFrame(const QByteArray &frame);

    QScopedPointer<QTcpSocket> m_pSocket;
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
    QWebSocketProtocol::CloseCode m_closeCode;
    QString m_closeReason;

    QTime m_pingTimer;

    QWebSocketDataProcessor m_dataProcessor;
    QWebSocketConfiguration m_configuration;

    friend class QWebSocketServerPrivate;
};

QT_END_NAMESPACE

#endif // QWEBSOCKET_H
