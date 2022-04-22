/****************************************************************************
**
** Copyright (C) 2022 Menlo Systems GmbH, author Arno Rehn <a.rehn@menlosystems.com>.
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

#include "qwebsockethandshakeoptions_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebSocketHandshakeOptions

    \inmodule QtWebSockets
    \since 6.4
    \brief Collects options for the WebSocket handshake.

    QWebSocketHandshakeOptions collects options that are passed along to the
    WebSocket handshake, such as WebSocket subprotocols and WebSocket
    Extensions.

    At the moment, only WebSocket subprotocols are supported.

    \sa QWebSocket::open()
*/

/*!
    \brief Constructs an empty QWebSocketHandshakeOptions object.
*/
QWebSocketHandshakeOptions::QWebSocketHandshakeOptions()
    : d(new QWebSocketHandshakeOptionsPrivate)
{
}

/*!
    \brief Constructs a QWebSocketHandshakeOptions that is a copy of \a other.
*/
QWebSocketHandshakeOptions::QWebSocketHandshakeOptions(const QWebSocketHandshakeOptions &other)
    : d(other.d)
{
}

/*!
    \brief Constructs a QWebSocketHandshakeOptions that is moved from \a other.
*/
QWebSocketHandshakeOptions::QWebSocketHandshakeOptions(QWebSocketHandshakeOptions &&other) noexcept
    : d(std::move(other.d))
{
}

/*!
    \brief Destroys this object.
*/
QWebSocketHandshakeOptions::~QWebSocketHandshakeOptions()
{
}

/*!
    \fn QWebSocketHandshakeOptions &QWebSocketHandshakeOptions::operator=(QWebSocketHandshakeOptions &&other)
    \brief Moves \a other to this object.
*/

/*!
    \brief Assigns \a other to this object.
*/
QWebSocketHandshakeOptions &QWebSocketHandshakeOptions::operator=(
        const QWebSocketHandshakeOptions &other)
{
    d = other.d;
    return *this;
}

/*!
    \fn void swap(QWebSocketHandshakeOptions &other) noexcept
    \brief Swaps this object with \a other.
*/

/*!
    \brief Returns the list of WebSocket subprotocols to send along with the
           websocket handshake.
*/
QStringList QWebSocketHandshakeOptions::subprotocols() const
{
    return d->subprotocols;
}

/*!
    \brief Sets the list of WebSocket subprotocols \a protocols to send along
           with the websocket handshake.

    WebSocket subprotocol names may only consist of those US-ASCII characters
    that are in the unreserved group. Invalid protocol names will not be
    included in the handshake.
*/
void QWebSocketHandshakeOptions::setSubprotocols(const QStringList &protocols)
{
    d->subprotocols = protocols;
}

bool QWebSocketHandshakeOptions::equals(const QWebSocketHandshakeOptions &other) const
{
    return *d == *other.d;
}

/*!
    //! friend
    \fn QWebSocketHandshakeOptions::operator==(const QWebSocketHandshakeOptions &lhs, const QWebSocketHandshakeOptions &rhs)
    \fn QWebSocketHandshakeOptions::operator!=(const QWebSocketHandshakeOptions &lhs, const QWebSocketHandshakeOptions &rhs)
    \brief Compares \a lhs for equality with \a rhs.
*/
QT_END_NAMESPACE
