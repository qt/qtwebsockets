/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "qwebsocket_p.h"

#include <QtCore/qcoreapplication.h>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/bind.h>

using namespace emscripten;

QByteArray g_messageArray;
// easiest way to transliterate binary data to js/wasm

val getBinaryMessage()
{
    return val(typed_memory_view(g_messageArray.size(),
                                 reinterpret_cast<const unsigned char *>(g_messageArray.constData())));
}

EMSCRIPTEN_BINDINGS(wasm_module) {
    function("getBinaryMessage", &getBinaryMessage);
}

static void onOpenCallback(void *data)
{
    auto handler = reinterpret_cast<QWebSocketPrivate *>(data);
    Q_ASSERT (handler);
    emit handler->q_func()->connected();
}

static void onCloseCallback(void *data, int message)
{
    Q_UNUSED(message);
    auto handler = reinterpret_cast<QWebSocketPrivate *>(data);
    Q_ASSERT (handler);
    emit handler->q_func()->disconnected();
}

static void onErrorCallback(void *data, int message)
{
    Q_UNUSED(message);
    auto handler = reinterpret_cast<QWebSocketPrivate *>(data);
    Q_ASSERT (handler);
    emit handler->q_func()->error(handler->error());
}

static void onIncomingMessageCallback(void *data, int message, int length, int dataType)
{
    QWebSocketPrivate *handler = reinterpret_cast<QWebSocketPrivate *>(data);
    Q_ASSERT (handler);

    QWebSocket *webSocket = handler->q_func();
    const char *text = reinterpret_cast<const char *>(message);

    switch (dataType) {
    case 0: //string
        webSocket->textMessageReceived(QLatin1String(text));
        break;
    case 1: //blob
    case 2: //arraybuffer
        webSocket->binaryMessageReceived(QByteArray::fromRawData(text, length));
        break;
    };
}

qint64 QWebSocketPrivate::sendTextMessage(const QString &message)
{
    EM_ASM_ARGS({
        if (window.qWebSocket === undefined)
            console.log("cannot find websocket object");
        else
            window.qWebSocket.send(Pointer_stringify($0));
     }, message.toLocal8Bit().constData());

    return message.length();
}

qint64 QWebSocketPrivate::sendBinaryMessage(const QByteArray &data)
{
    g_messageArray = data;
    EM_ASM({
        if (window.qWebSocket === undefined) {
            console.log("cannot find websocket object");
        } else {
            var array = Module.getBinaryMessage();
            window.qWebSocket.binaryType = 'arraybuffer';
            window.qWebSocket.send(array);
        }
    });

    g_messageArray.clear();
    return data.length();
}

void QWebSocketPrivate::close(QWebSocketProtocol::CloseCode closeCode, QString reason)
{
    Q_Q(QWebSocket);
    m_closeCode = closeCode;
    m_closeReason = reason;
    const quint16 closeReason = (quint16)closeCode;
    Q_EMIT q->aboutToClose();
    QCoreApplication::processEvents();

    EM_ASM_ARGS({
        if (window.qWebSocket === undefined) {
            console.log("cannot find websocket object");
        } else {
            var reasonMessage = Pointer_stringify($0);
            window.qWebSocket.close($1, reasonMessage);
            window.qWebSocket = undefined;
        }
    }, reason.toLatin1().data(), closeReason);
}

void QWebSocketPrivate::open(const QNetworkRequest &request, bool mask)
{
    Q_UNUSED(mask)
    Q_Q(QWebSocket);
    const QUrl url = request.url();
    if (!url.isValid() || url.toString().contains(QStringLiteral("\r\n"))) {
        setErrorString(QWebSocket::tr("Invalid URL."));
        Q_EMIT q->error(QAbstractSocket::ConnectionRefusedError);
        return;
    }

    QByteArray urlbytes = url.toString().toUtf8();

    // HTML WebSockets do not support arbitrary request headers, but
    // do support the WebSocket protocol header. This header is
    // required for some use cases like MQTT.
    QByteArray protocolHeaderValue = request.rawHeader("Sec-WebSocket-Protocol");

    EM_ASM_ARGS({
        if (window.qWebSocket != undefined)
            return;

        var wsUri = Pointer_stringify($0);
        var wsProtocol = Pointer_stringify($1);
        var handler = $2;
        var onOpenCb = $3;
        var onCloseCb = $4;
        var onErrorCb = $5;
        var onIncomingMessageCb = $6;

        window.qWebSocket = wsProtocol.length > 0
                           ? new WebSocket(wsUri, wsProtocol)
                           : new WebSocket(wsUri);

        window.qWebSocket.onopen = function(event) {
            Runtime.dynCall('vi', onOpenCb, [handler]);
        };

        window.qWebSocket.onclose = function(event) {
            window.qWebSocket = undefined;
            Runtime.dynCall('vii', onCloseCb, [handler, event.code]);
        };

        window.qWebSocket.onerror = function(event) {
            Runtime.dynCall('vii', onErrorCb, [handler, event.error]);
        };

        window.qWebSocket.onmessage = function(event) {
            var outgoingMessage;
            var bufferLength = 0;
            var dataType;

            if (window.qWebSocket.binaryType == 'arraybuffer' && typeof event.data == 'object') {

                var byteArray = new Uint8Array(event.data);
                bufferLength = byteArray.length;

                outgoingMessage = _malloc(byteArray.length);
                HEAPU8.set(byteArray, outgoingMessage);

                dataType = 2;
            } else if (typeof event.data == 'string') {
                 dataType = 0;
                 outgoingMessage = allocate(intArrayFromString(event.data), 'i8', ALLOC_NORMAL);
            } else if (window.qWebSocket.binaryType == 'blob') {
                 var byteArray = new Int8Array($0);
                 outgoingMessage = new Blob(byteArray.buffer);
                 dataType = 1;
            }

            Runtime.dynCall('viiii', onIncomingMessageCb, [handler, outgoingMessage, bufferLength, dataType]);
            _free(outgoingMessage);
        };

    }, urlbytes.constData(),
    protocolHeaderValue.data(),
    this,
    reinterpret_cast<void *>(onOpenCallback),
    reinterpret_cast<void *>(onCloseCallback),
    reinterpret_cast<void *>(onErrorCallback),
    reinterpret_cast<void *>(onIncomingMessageCallback));
}
