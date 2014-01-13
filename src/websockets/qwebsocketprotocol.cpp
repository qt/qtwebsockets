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

#include "qwebsocketprotocol_p.h"
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QtEndian>

QT_BEGIN_NAMESPACE

/*!
  \namespace QWebSocketProtocol
  \inmodule QtWebSockets
  \brief Contains constants related to the WebSocket standard.
*/

/*!
    \enum QWebSocketProtocol::CloseCode

    \inmodule QtWebSockets

    The close codes supported by WebSockets V13

    \value CC_NORMAL                    Normal closure
    \value CC_GOING_AWAY                Going away
    \value CC_PROTOCOL_ERROR            Protocol error
    \value CC_DATATYPE_NOT_SUPPORTED    Unsupported data
    \value CC_RESERVED_1004             Reserved
    \value CC_MISSING_STATUS_CODE       No status received
    \value CC_ABNORMAL_DISCONNECTION    Abnormal closure
    \value CC_WRONG_DATATYPE            Invalid frame payload data
    \value CC_POLICY_VIOLATED           Policy violation
    \value CC_TOO_MUCH_DATA             Message too big
    \value CC_MISSING_EXTENSION         Mandatory extension missing
    \value CC_BAD_OPERATION             Internal server error
    \value CC_TLS_HANDSHAKE_FAILED      TLS handshake failed

    \sa QWebSocket::close()
*/
/*!
    \enum QWebSocketProtocol::Version

    \inmodule QtWebSockets

    \brief The different defined versions of the Websocket protocol.

    For an overview of the differences between the different protocols, see
    <http://code.google.com/p/pywebsocket/wiki/WebSocketProtocolSpec>

    \value V_Unknow
    \value V_0        hixie76: http://tools.ietf.org/html/draft-hixie-thewebsocketprotocol-76 &
                      hybi-00: http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-00.
                      Works with key1, key2 and a key in the payload.
                      Attribute: Sec-WebSocket-Draft value 0.
    \value V_4        hybi-04: http://tools.ietf.org/id/draft-ietf-hybi-thewebsocketprotocol-04.txt.
                      Changed handshake: key1, key2, key3
                      ==> Sec-WebSocket-Key, Sec-WebSocket-Nonce, Sec-WebSocket-Accept
                      Sec-WebSocket-Draft renamed to Sec-WebSocket-Version
                      Sec-WebSocket-Version = 4
    \value V_5        hybi-05: http://tools.ietf.org/id/draft-ietf-hybi-thewebsocketprotocol-05.txt.
                      Sec-WebSocket-Version = 5
                      Removed Sec-WebSocket-Nonce
                      Added Sec-WebSocket-Accept
    \value V_6        Sec-WebSocket-Version = 6.
    \value V_7        hybi-07: http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-07.
                      Sec-WebSocket-Version = 7
    \value V_8        hybi-8, hybi-9, hybi-10, hybi-11 and hybi-12.
                      Status codes 1005 and 1006 are added and all codes are now unsigned
                      Internal error results in 1006
    \value V_13       hybi-13, hybi14, hybi-15, hybi-16, hybi-17 and RFC 6455.
                      Sec-WebSocket-Version = 13
                      Status code 1004 is now reserved
                      Added 1008, 1009 and 1010
                      Must support TLS
                      Clarify multiple version support
    \value V_LATEST   Refers to the latest know version to QWebSockets.
*/

/*!
    \enum QWebSocketProtocol::OpCode

    \inmodule QtWebSockets

    The frame opcodes as defined by the WebSockets standard

    \value OC_CONTINUE      Continuation frame
    \value OC_TEXT          Text frame
    \value OC_BINARY        Binary frame
    \value OC_RESERVED_3    Reserved
    \value OC_RESERVED_4    Reserved
    \value OC_RESERVED_5    Reserved
    \value OC_RESERVED_6    Reserved
    \value OC_RESERVED_7    Reserved
    \value OC_CLOSE         Close frame
    \value OC_PING          Ping frame
    \value OC_PONG          Pong frame
    \value OC_RESERVED_B    Reserved
    \value OC_RESERVED_C    Reserved
    \value OC_RESERVED_D    Reserved
    \value OC_RESERVED_E    Reserved
    \value OC_RESERVED_F    Reserved

    \internal
*/

/*!
  \fn QWebSocketProtocol::isOpCodeReserved(OpCode code)
  Checks if \a code is a valid OpCode

  \internal
*/

/*!
  \fn QWebSocketProtocol::isCloseCodeValid(int closeCode)
  Checks if \a closeCode is a valid web socket close code

  \internal
*/

/*!
  \fn QWebSocketProtocol::currentVersion()
  Returns the latest version that WebSocket is supporting

  \internal
*/

/*!
    Parses the \a versionString and converts it to a Version value

    \internal
*/
QWebSocketProtocol::Version QWebSocketProtocol::versionFromString(const QString &versionString)
{
    bool ok = false;
    Version version = V_Unknow;
    const int ver = versionString.toInt(&ok);
    QSet<Version> supportedVersions;
    supportedVersions << V_0 << V_4 << V_5 << V_6 << V_7 << V_8 << V_13;
    if (Q_LIKELY(ok) && (supportedVersions.contains(static_cast<Version>(ver))))
        version = static_cast<Version>(ver);
    return version;
}

/*!
    Mask the \a payload with the given \a maskingKey and stores the result back in \a payload.

    \internal
*/
void QWebSocketProtocol::mask(QByteArray *payload, quint32 maskingKey)
{
    Q_ASSERT(payload);
    mask(payload->data(), payload->size(), maskingKey);
}

/*!
    Masks the \a payload of length \a size with the given \a maskingKey and
    stores the result back in \a payload.

    \internal
*/
void QWebSocketProtocol::mask(char *payload, quint64 size, quint32 maskingKey)
{
    Q_ASSERT(payload);
    const quint8 mask[] = { quint8((maskingKey & 0xFF000000u) >> 24),
                            quint8((maskingKey & 0x00FF0000u) >> 16),
                            quint8((maskingKey & 0x0000FF00u) >> 8),
                            quint8((maskingKey & 0x000000FFu))
                          };
    int i = 0;
    while (size-- > 0)
        *payload++ ^= mask[i++ % 4];
}

QT_END_NAMESPACE
