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

#include "qwebsocket.h"
#include "qwebsocket_wasm_p.h"
#include "qwebsocketprotocol_p.h"
#include "qwebsockethandshakerequest_p.h"
#include "qwebsockethandshakeresponse_p.h"
#include "qdefaultmaskgenerator_p.h"

#include <QtCore/QUrl>
#include <QtCore/QByteArray>
#include <QtCore/QCryptographicHash>
#include <QtCore/QRegularExpression>
#include <QtCore/QStringList>

#include <QtCore/QStringBuilder>   //for more efficient string concatenation

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <limits>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/bind.h>

using namespace emscripten;
QT_BEGIN_NAMESPACE

QByteArray m_messageArray;

val getBinaryMessage()
{
    return val(typed_memory_view(m_messageArray.size(), reinterpret_cast<const unsigned char *>(m_messageArray.constData())));
}

EMSCRIPTEN_BINDINGS(wasm_module) {
    function("getBinaryMessage", &getBinaryMessage);
}

QWebSocketConfiguration::QWebSocketConfiguration()
{
}

/*!
    \internal
*/
QWebSocketPrivate::QWebSocketPrivate(const QString &origin, QWebSocketProtocol::Version version,
                                     QWebSocket *pWebSocket) :
    QObjectPrivate(),
    q_ptr(pWebSocket),
    m_errorString(),
    m_version(version),
    m_resourceName(),
    m_origin(origin),
    m_request(),
    m_protocol(),
    m_extension(),
    m_socketState(QAbstractSocket::UnconnectedState),
    m_pauseMode(QAbstractSocket::PauseNever),
    m_readBufferSize(0),
    m_key(),
    m_mustMask(true),
    m_isClosingHandshakeSent(false),
    m_isClosingHandshakeReceived(false),
    m_closeCode(QWebSocketProtocol::CloseCodeNormal),
    m_closeReason(),
    m_pingTimer(),
    m_dataProcessor(),
    m_configuration(),
    m_pMaskGenerator(&m_defaultMaskGenerator),
    m_defaultMaskGenerator(),
    m_handshakeState(NothingDoneState)
{
}

/*!
    \internal
*/
QWebSocketPrivate::QWebSocketPrivate(QTcpSocket *pTcpSocket, QWebSocketProtocol::Version version,
                                     QWebSocket *pWebSocket) :
    QObjectPrivate(),
    q_ptr(pWebSocket),
    m_errorString(),
    m_version(version),
    m_resourceName(),
    m_origin(),
    m_request(),
    m_protocol(),
    m_extension(),
    m_socketState(QAbstractSocket::UnconnectedState),
    m_pauseMode(QAbstractSocket::PauseNever),
    m_readBufferSize(0),
    m_key(),
    m_mustMask(true),
    m_isClosingHandshakeSent(false),
    m_isClosingHandshakeReceived(false),
    m_closeCode(QWebSocketProtocol::CloseCodeNormal),
    m_closeReason(),
    m_pingTimer(),
    m_dataProcessor(),
    m_configuration(),
    m_pMaskGenerator(&m_defaultMaskGenerator),
    m_defaultMaskGenerator(),
    m_handshakeState(NothingDoneState)
{
    Q_UNUSED(pTcpSocket)
}
/*!
    \internal
*/
void QWebSocketPrivate::init()
{
    Q_ASSERT(q_ptr);
    Q_ASSERT(m_pMaskGenerator);

    m_pMaskGenerator->seed();
    makeConnections();
}

/*!
    \internal
*/
QWebSocketPrivate::~QWebSocketPrivate()
{
}

/*!
    \internal
*/
void QWebSocketPrivate::closeGoingAway()
{
    if (state() == QAbstractSocket::ConnectedState)
        close(QWebSocketProtocol::CloseCodeGoingAway, QWebSocket::tr("Connection closed"));
    releaseConnections();
}

/*!
    \internal
 */
void QWebSocketPrivate::abort()
{

}

/*!
    \internal
 */
QAbstractSocket::SocketError QWebSocketPrivate::error() const
{
    QAbstractSocket::SocketError err = m_lastError;
    return err;
}

/*!
    \internal
 */
QString QWebSocketPrivate::errorString() const
{
    QString errMsg;
    if (!m_errorString.isEmpty())
        errMsg = m_errorString;
    return errMsg;
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::sendTextMessage(const QString &message)
{
    EM_ASM_ARGS({
                    var wsMessage = Pointer_stringify($0);
                    if (window.webSocker == undefined) {
                        console.log("cannot find websocket object");
                    } else {
                        window.webSocker.send(wsMessage);
                    }
                }, message.toLocal8Bit().constData()
                );

    return message.length();
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::sendBinaryMessage(const QByteArray &data)
{
    m_messageArray = data;

    EM_ASM_ARGS({
                    var array = Module.getBinaryMessage();
                    if (window.webSocker == undefined) {
                        console.log("cannot find websocket object");
                    } else {
                        window.webSocker.binaryType = "arraybuffer";
                        window.webSocker.send(array);
                    }
                }, data.constData()
                );

    return data.length();
}

/*!
  Called from QWebSocketServer
  \internal
 */
QWebSocket *QWebSocketPrivate::upgradeFrom(QTcpSocket *pTcpSocket,
                                           const QWebSocketHandshakeRequest &request,
                                           const QWebSocketHandshakeResponse &response,
                                           QObject *parent)
{
    QWebSocket *pWebSocket = new QWebSocket(pTcpSocket, response.acceptedVersion(), parent);
    if (Q_LIKELY(pWebSocket)) {
        QNetworkRequest netRequest(request.requestUrl());
        const auto headers = request.headers();
        for (auto it = headers.begin(), end = headers.end(); it != end; ++it)
            netRequest.setRawHeader(it.key().toLatin1(), it.value().toLatin1());

        pWebSocket->d_func()->setExtension(response.acceptedExtension());
        pWebSocket->d_func()->setOrigin(request.origin());
        pWebSocket->d_func()->setRequest(netRequest);
        pWebSocket->d_func()->setProtocol(response.acceptedProtocol());
        pWebSocket->d_func()->setResourceName(request.requestUrl().toString(QUrl::RemoveUserInfo));
        //a server should not send masked frames
        pWebSocket->d_func()->enableMasking(false);
    }

    return pWebSocket;
}

/*!
    \internal
 */
void QWebSocketPrivate::close(QWebSocketProtocol::CloseCode closeCode, QString reason)
{
    Q_Q(QWebSocket);
    m_closeCode = closeCode;
    m_closeReason = reason;
    const quint16 closeReason = (quint16)closeCode;
    Q_EMIT q->aboutToClose();
    QCoreApplication::processEvents();

    EM_ASM_ARGS({
                    var reasonMessage = Pointer_stringify($0);
                    var closingCode = $1;

                    if (window.webSocker == undefined) {
                        console.log("cannot find websocket object");
                    } else {
                        window.webSocker.close(closingCode,reasonMessage);
                    }
                }, reason.toLatin1().data(),
                closeReason
                );
}

/*!
    \internal
 */
void QWebSocketPrivate::onOpenCallback(void *data)
{
    QWebSocketPrivate *handler = reinterpret_cast<QWebSocketPrivate*>(data);
    if (handler)
        handler->emitConnected();
    QCoreApplication::processEvents();
}

/*!
    \internal
 */
void QWebSocketPrivate::emitConnected()
{
    Q_Q(QWebSocket);
    Q_EMIT q->connected();
    QCoreApplication::processEvents();
}

/*!
    \internal
 */
void QWebSocketPrivate::onErrorCallback(void *data, int evt)
{
    QWebSocketPrivate *handler = reinterpret_cast<QWebSocketPrivate*>(data);

    if (handler)
        handler->emitErrorReceived(evt);
}

/*!
    \internal
 */
void QWebSocketPrivate::emitErrorReceived(int errorEvt)
{
    m_lastError = static_cast<QAbstractSocket::SocketError>(errorEvt);
    Q_Q(QWebSocket);
    Q_EMIT q->error(m_lastError);
    QCoreApplication::processEvents();
}

/*!
    \internal
 */
void QWebSocketPrivate::onCloseCallback(void *data, int /*evt*/)
{
    QWebSocketPrivate *handler = reinterpret_cast<QWebSocketPrivate*>(data);

    if (handler)
        handler->emitDisconnected();
}

/*!
    \internal
 */
void QWebSocketPrivate::emitDisconnected()
{
    Q_Q(QWebSocket);
    Q_EMIT q->disconnected();
    QCoreApplication::processEvents();
}

/*!
    \internal
 */
void QWebSocketPrivate::onIncomingMessageCallback(void *data, int evt, int length, int dataType)
{
    QWebSocketPrivate *handler = reinterpret_cast<QWebSocketPrivate*>(data);
    if (handler) {
        if (dataType == 0) { //string
            QLatin1String message((char *)evt);
            handler->emitTextMessageReceived(message);
        } else if (dataType == 1) {//blob ??
            handler->emitBinaryMessageReceived(QByteArray::fromRawData((char *)evt, length));
        } else  if (dataType == 2) {//arraybuffer
            handler->emitBinaryMessageReceived(QByteArray::fromRawData((char *)evt, length));
        }
    } else {
        //error
    }
}

/*!
    \internal
 */
void QWebSocketPrivate::emitTextMessageReceived(const QString &message)
{
    Q_Q(QWebSocket);
    Q_EMIT q->textMessageReceived(message);
    QCoreApplication::processEvents();
}

/*!
    \internal
 */
void QWebSocketPrivate::emitBinaryMessageReceived(const QByteArray &message)
{
    Q_Q(QWebSocket);
    Q_EMIT q->binaryMessageReceived(message);
    QCoreApplication::processEvents();
}

/*!
    \internal
 */
void QWebSocketPrivate::open(const QNetworkRequest &request, bool mask)
{
    Q_UNUSED(mask)
    Q_Q(QWebSocket);
    QUrl url = request.url();
    if (!url.isValid() || url.toString().contains(QStringLiteral("\r\n"))) {
        setErrorString(QWebSocket::tr("Invalid URL."));
        Q_EMIT q->error(QAbstractSocket::ConnectionRefusedError);
        return;
    }
    QList <QByteArray> headerList = request.rawHeaderList();
    if (headerList.count() > 0) {

    }
    jsRequest(url.toString(),
              (void *)&onOpenCallback,
              (void *)&onCloseCallback,
              (void *)&onErrorCallback,
              (void *)&onIncomingMessageCallback);
}

void QWebSocketPrivate::jsRequest(const QString &url, void *openCallback, void *closeCallback, void *errorCallback, void *incomingMessageCallback)
{
    EM_ASM_ARGS({
                    var wsUri = Pointer_stringify($0);
                    var handler = $1;
                    var onOpenCallbackPointer = $2;
                    var onCloseCallback = $3;
                    var onErrorCallback = $4;
                    var onIncomingMessageCallback = $5;

                    function connectWebSocket() {
                        if (window.webSocker == undefined) {
                            window.webSocker = {};

                            window.webSocker = new WebSocket(wsUri);

                            window.webSocker.onopen = onOpen;
                            window.webSocker.onclose = onClose;
                            window.webSocker.onmessage = onMessage;
                            window.webSocker.onerror = onError;
                        }
                    }

                    function onOpen(evt) {
                        Runtime.dynCall('vi', onOpenCallbackPointer, [handler]);
                    }

                    function onClose(evt) {
                        window.webSocker = {};
                        Runtime.dynCall('vii', onCloseCallback, [handler, evt.code]);
                    }

                    function onError(evt) {
                        Runtime.dynCall('vii', onErrorCallback, [handler, evt.error]);
                    }

                    function onMessage(evt) {

                        var ptr;
                        var bufferLength = 0;
                        var dataType;
                        if (window.webSocker.binaryType == "arraybuffer" && typeof evt.data == "object") {

                            var byteArray = new Uint8Array(evt.data);
                            bufferLength = byteArray.length;

                            ptr = _malloc(byteArray.length);
                            HEAP8.set(byteArray, ptr);

                            dataType = 2;
                        }
                        else if (window.webSocker.binaryType == "blob") {
                            var byteArray = new Int8Array($0);
                            console.log("type blob");
                            ptr = new Blob(byteArray.buffer);
                            dataType = 1;
                        }
                        else if (typeof evt.data == "string") {
                            dataType = 0;
                            ptr = allocate(intArrayFromString(evt.data), 'i8', ALLOC_NORMAL);
                        }

                        Runtime.dynCall('viiii', onIncomingMessageCallback, [handler, ptr, bufferLength, dataType]);
                        _free(ptr);
                    }

                    connectWebSocket();

                }, url.toLatin1().data(),
                this,
                (void *)openCallback,
                (void *)closeCallback,
                (void *)errorCallback,
                (void *)incomingMessageCallback
                );
    QCoreApplication::processEvents();
}

/*!
    \internal
 */
void QWebSocketPrivate::ping(const QByteArray &payload)
{
    QByteArray payloadTruncated = payload.left(125);
    m_pingTimer.restart();
    quint32 maskingKey = 0;
    if (m_mustMask)
        maskingKey = generateMaskingKey();
    QByteArray pingFrame = getFrameHeader(QWebSocketProtocol::OpCodePing, payloadTruncated.size(),
                                          maskingKey, true);

    if (m_mustMask)
        QWebSocketProtocol::mask(&payloadTruncated, maskingKey);
    pingFrame.append(payloadTruncated);
    qint64 ret = sendBinaryMessage(pingFrame);
    Q_UNUSED(ret);
}

/*!
  \internal
    Sets the version to use for the WebSocket protocol;
    this must be set before the socket is opened.
*/
void QWebSocketPrivate::setVersion(QWebSocketProtocol::Version version)
{
    if (m_version != version)
        m_version = version;
}

/*!
    \internal
    Sets the resource name of the connection; must be set before the socket is openend
*/
void QWebSocketPrivate::setResourceName(const QString &resourceName)
{
    if (m_resourceName != resourceName)
        m_resourceName = resourceName;
}

/*!
  \internal
 */
void QWebSocketPrivate::setRequest(const QNetworkRequest &request)
{
    if (m_request != request)
        m_request = request;
}

/*!
  \internal
 */
void QWebSocketPrivate::setOrigin(const QString &origin)
{
    if (m_origin != origin)
        m_origin = origin;
}

/*!
  \internal
 */
void QWebSocketPrivate::setProtocol(const QString &protocol)
{
    if (m_protocol != protocol)
        m_protocol = protocol;
}

/*!
  \internal
 */
void QWebSocketPrivate::setExtension(const QString &extension)
{
    if (m_extension != extension)
        m_extension = extension;
}

/*!
  \internal
 */
void QWebSocketPrivate::enableMasking(bool enable)
{
    if (m_mustMask != enable)
        m_mustMask = enable;
}

/*!
 * \internal
 */
void QWebSocketPrivate::makeConnections()
{
    Q_Q(QWebSocket);

    QObject::connect(&m_dataProcessor, &QWebSocketDataProcessor::textFrameReceived, q,
                     &QWebSocket::textFrameReceived);
    QObject::connect(&m_dataProcessor, &QWebSocketDataProcessor::binaryFrameReceived, q,
                     &QWebSocket::binaryFrameReceived);
    QObject::connect(&m_dataProcessor, &QWebSocketDataProcessor::binaryMessageReceived, q,
                     &QWebSocket::binaryMessageReceived);
    QObject::connect(&m_dataProcessor, &QWebSocketDataProcessor::textMessageReceived, q,
                     &QWebSocket::textMessageReceived);
    QObjectPrivate::connect(&m_dataProcessor, &QWebSocketDataProcessor::errorEncountered, this,
                            &QWebSocketPrivate::close);
    QObjectPrivate::connect(&m_dataProcessor, &QWebSocketDataProcessor::closeReceived, this,
                            &QWebSocketPrivate::processClose);
}

/*!
 * \internal
 */
void QWebSocketPrivate::releaseConnections()
{

    m_dataProcessor.disconnect();
}

/*!
    \internal
 */
QWebSocketProtocol::Version QWebSocketPrivate::version() const
{
    return m_version;
}

/*!
    \internal
 */
QString QWebSocketPrivate::resourceName() const
{
    return m_resourceName;
}

/*!
    \internal
 */
QNetworkRequest QWebSocketPrivate::request() const
{
    return m_request;
}

/*!
    \internal
 */
QString QWebSocketPrivate::origin() const
{
    return m_origin;
}

/*!
    \internal
 */
QString QWebSocketPrivate::protocol() const
{
    return m_protocol;
}

/*!
    \internal
 */
QString QWebSocketPrivate::extension() const
{
    return m_extension;
}

/*!
 * \internal
 */
QWebSocketProtocol::CloseCode QWebSocketPrivate::closeCode() const
{
    return m_closeCode;
}

/*!
 * \internal
 */
QString QWebSocketPrivate::closeReason() const
{
    return m_closeReason;
}

///*!
// * \internal
// */
QByteArray QWebSocketPrivate::getFrameHeader(QWebSocketProtocol::OpCode opCode,
                                             quint64 payloadLength, quint32 maskingKey,
                                             bool lastFrame)
{
    QByteArray header;
    bool ok = payloadLength <= 0x7FFFFFFFFFFFFFFFULL;

    if (Q_LIKELY(ok)) {
        //FIN, RSV1-3, opcode (RSV-1, RSV-2 and RSV-3 are zero)
        quint8 byte = static_cast<quint8>((opCode & 0x0F) | (lastFrame ? 0x80 : 0x00));
        header.append(static_cast<char>(byte));

        byte = 0x00;
        if (maskingKey != 0)
            byte |= 0x80;
        if (payloadLength <= 125) {
            byte |= static_cast<quint8>(payloadLength);
            header.append(static_cast<char>(byte));
        } else if (payloadLength <= 0xFFFFU) {
            byte |= 126;
            header.append(static_cast<char>(byte));
            header.append(static_cast<const char *>(static_cast<const void *>(&payloadLength)), 2);
        } else if (payloadLength <= 0x7FFFFFFFFFFFFFFFULL) {
            byte |= 127;
            header.append(static_cast<char>(byte));
            header.append(static_cast<const char *>(static_cast<const void *>(&payloadLength)), 8);
        }

        if (maskingKey != 0) {
            header.append(static_cast<const char *>(static_cast<const void *>(&maskingKey)),
                          sizeof(quint32));
        }
    } else {
        setErrorString(QStringLiteral("WebSocket::getHeader: payload too big!"));
        Q_EMIT q_ptr->error(QAbstractSocket::DatagramTooLargeError);
    }

    return header;
}


/*!
    \internal
 */
quint32 QWebSocketPrivate::generateMaskingKey() const
{
    return m_pMaskGenerator->nextMask();
}

/*!
    \internal
 */
QByteArray QWebSocketPrivate::generateKey() const
{
    QByteArray key;

    for (int i = 0; i < 4; ++i) {
        const quint32 tmp = m_pMaskGenerator->nextMask();
        key.append(static_cast<const char *>(static_cast<const void *>(&tmp)), sizeof(quint32));
    }

    return key.toBase64();
}


/*!
    \internal
 */
QString QWebSocketPrivate::calculateAcceptKey(const QByteArray &key) const
{
    const QByteArray tmpKey = key + QByteArrayLiteral("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    const QByteArray hash = QCryptographicHash::hash(tmpKey, QCryptographicHash::Sha1).toBase64();
    return QString::fromLatin1(hash);
}

/*!
 \internal
 */
void QWebSocketPrivate::processClose(QWebSocketProtocol::CloseCode closeCode, QString closeReason)
{
    m_isClosingHandshakeReceived = true;
    close(closeCode, closeReason);
}

/*!
    \internal
 */
QAbstractSocket::SocketState QWebSocketPrivate::state() const
{
    return m_socketState;
}

/*!
    \internal
 */
void QWebSocketPrivate::setSocketState(QAbstractSocket::SocketState state)
{
    Q_Q(QWebSocket);
    if (m_socketState != state) {
        m_socketState = state;
        Q_EMIT q->stateChanged(m_socketState);
        QCoreApplication::processEvents();
    }
}

/*!
    \internal
 */
void QWebSocketPrivate::setErrorString(const QString &errorString)
{
    if (m_errorString != errorString)
        m_errorString = errorString;
}

/*!
    \internal
 */
QHostAddress QWebSocketPrivate::localAddress() const
{
    QHostAddress address;
    return address;
}

/*!
    \internal
 */
quint16 QWebSocketPrivate::localPort() const
{
    quint16 port = 0;
    return port;
}

/*!
    \internal
 */
QAbstractSocket::PauseModes QWebSocketPrivate::pauseMode() const
{
    return m_pauseMode;
}

/*!
    \internal
 */
QHostAddress QWebSocketPrivate::peerAddress() const
{
    QHostAddress address;
    return address;
}

/*!
    \internal
 */
QString QWebSocketPrivate::peerName() const
{
    QString name;
    return name;
}

/*!
    \internal
 */
quint16 QWebSocketPrivate::peerPort() const
{
    quint16 port = 0;
    return port;
}


/*!
    \internal
 */
void QWebSocketPrivate::setMaskGenerator(const QMaskGenerator *maskGenerator)
{
    if (!maskGenerator)
        m_pMaskGenerator = &m_defaultMaskGenerator;
    else if (maskGenerator != m_pMaskGenerator)
        m_pMaskGenerator = const_cast<QMaskGenerator *>(maskGenerator);
}

/*!
    \internal
 */
const QMaskGenerator *QWebSocketPrivate::maskGenerator() const
{
    Q_ASSERT(m_pMaskGenerator);
    return m_pMaskGenerator;
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::readBufferSize() const
{
    return m_readBufferSize;
}

/*!
    \internal
 */
void QWebSocketPrivate::resume()
{
}

/*!
  \internal
 */
void QWebSocketPrivate::setPauseMode(QAbstractSocket::PauseModes pauseMode)
{
    m_pauseMode = pauseMode;
}

/*!
    \internal
 */
void QWebSocketPrivate::setReadBufferSize(qint64 size)
{
    m_readBufferSize = size;
}

/*!
    \internal
 */
bool QWebSocketPrivate::isValid() const
{
    return true;
}

QT_END_NAMESPACE
