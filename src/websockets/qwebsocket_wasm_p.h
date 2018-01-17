/****************************************************************************
**
** Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWEBSOCKET_WASM_P_H
#define QWEBSOCKET_WASM_P_H
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
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkRequest>

#include <QtCore/QTime>
#include <private/qobject_p.h>

#include "qwebsocketprotocol.h"
#include "qwebsocketdataprocessor_p.h"
#include "qdefaultmaskgenerator_p.h"

QT_BEGIN_NAMESPACE

class QWebSocketHandshakeRequest;
class QWebSocketHandshakeResponse;
class QWebSocket;
class QMaskGenerator;

struct QWebSocketConfiguration
{
    Q_DISABLE_COPY(QWebSocketConfiguration)

public:
    QWebSocketConfiguration();

public:

};

class QWebSocketPrivate : public QObjectPrivate
{
    Q_DISABLE_COPY(QWebSocketPrivate)

public:
    Q_DECLARE_PUBLIC(QWebSocket)
    explicit QWebSocketPrivate(const QString &origin,
                               QWebSocketProtocol::Version version,
                               QWebSocket * const pWebSocket);
    virtual ~QWebSocketPrivate();

    void init();
    void abort();
    QAbstractSocket::SocketError error() const;
    QString errorString() const;
    bool flush() {return true;}
    bool isValid() const;
    QHostAddress localAddress() const;
    quint16 localPort() const;
    QAbstractSocket::PauseModes pauseMode() const;
    QHostAddress peerAddress() const;
    QString peerName() const;
    quint16 peerPort() const;

    void setMaskGenerator(const QMaskGenerator *maskGenerator);
    const QMaskGenerator *maskGenerator() const;
    qint64 readBufferSize() const;
    void resume();
    void setPauseMode(QAbstractSocket::PauseModes pauseMode);
    void setReadBufferSize(qint64 size);
    QAbstractSocket::SocketState state() const;

    QWebSocketProtocol::Version version() const;
    QString resourceName() const;
    QNetworkRequest request() const;
    QString origin() const;
    QString protocol() const;
    QString extension() const;
    QWebSocketProtocol::CloseCode closeCode() const;
    QString closeReason() const;

    qint64 sendTextMessage(const QString &message);
    qint64 sendBinaryMessage(const QByteArray &data);

    void closeGoingAway();
    void close(QWebSocketProtocol::CloseCode closeCode, QString reason);
    void open(const QNetworkRequest &request, bool mask);
    void ping(const QByteArray &payload);

    QWebSocket * const q_ptr;

    static void onOpenCallback(void *data);
    static void onCloseCallback(void *data, int);
    static void onErrorCallback(void *data, int);
    static void onIncomingMessageCallback(void *data, int, int, int);

    void emitConnected();
    void emitDisconnected();
    void emitTextMessageReceived(const QString &message);
    void emitBinaryMessageReceived(const QByteArray &message);
    void emitErrorReceived(int);

private:

    QWebSocketPrivate(QTcpSocket *pTcpSocket, QWebSocketProtocol::Version version,
                      QWebSocket *pWebSocket);

    void setVersion(QWebSocketProtocol::Version version);
    void setResourceName(const QString &resourceName);
    void setRequest(const QNetworkRequest &request);
    void setOrigin(const QString &origin);
    void setProtocol(const QString &protocol);
    void setExtension(const QString &extension);
    void enableMasking(bool enable);
    void setSocketState(QAbstractSocket::SocketState state);
    void setErrorString(const QString &errorString);

    void socketDestroyed(QObject *socket);

    void processClose(QWebSocketProtocol::CloseCode closeCode, QString closeReason);
    void makeConnections();
    void releaseConnections();

    QByteArray getFrameHeader(QWebSocketProtocol::OpCode opCode, quint64 payloadLength,
                              quint32 maskingKey, bool lastFrame);
    QString calculateAcceptKey(const QByteArray &key) const;

    Q_REQUIRED_RESULT static QWebSocket *
    upgradeFrom(QTcpSocket *tcpSocket,
                const QWebSocketHandshakeRequest &request,
                const QWebSocketHandshakeResponse &response,
                QObject *parent = nullptr);

    quint32 generateMaskingKey() const;
    QByteArray generateKey() const;

    QString m_errorString;
    QWebSocketProtocol::Version m_version;
    QUrl m_resource;
    QString m_resourceName;
    QString m_origin;
    QNetworkRequest m_request;
    QString m_protocol;
    QString m_extension;
    QAbstractSocket::SocketState m_socketState;
    QAbstractSocket::PauseModes m_pauseMode;
    qint64 m_readBufferSize;

    QByteArray m_key; //identification key used in handshake requests

    bool m_mustMask; //a server must not mask the frames it sends

    bool m_isClosingHandshakeSent;
    bool m_isClosingHandshakeReceived;
    QWebSocketProtocol::CloseCode m_closeCode;
    QString m_closeReason;

    QTime m_pingTimer;

    QWebSocketDataProcessor m_dataProcessor;
    QWebSocketConfiguration m_configuration;

    QMaskGenerator *m_pMaskGenerator;
    QDefaultMaskGenerator m_defaultMaskGenerator;

    enum HandshakeState {
        NothingDoneState,
        ReadingStatusState,
        ReadingHeaderState,
        ParsingHeaderState,
        AllDoneState
    } m_handshakeState;
    QByteArray m_statusLine;
    int m_httpStatusCode;
    int m_httpMajorVersion, m_httpMinorVersion;
    QString m_httpStatusMessage;
    QMap<QString, QString> m_headers;

    friend class QWebSocketServerPrivate;

    void jsRequest(const QString &url, void *openCallback, void *closeCallback, void *errorCallback, void *incomingMessageCallback);
    QAbstractSocket::SocketError m_lastError;
};

QT_END_NAMESPACE

#endif // QWEBSOCKET_WASM_H
