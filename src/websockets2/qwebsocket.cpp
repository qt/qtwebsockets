/*
QWebSockets implements the WebSocket protocol as defined in RFC 6455.
Copyright (C) 2013 Kurt Pattyn (pattyn.kurt@gmail.com)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "qwebsocket.h"
#include "qwebsocket_p.h"

QWebSocket::QWebSocket(QObject *parent) :
    QObject(parent),
    d_ptr(new QWebSocketPrivate(this))
{ }

QWebSocket::QWebSocket(const QUrl &url, const QStringList &subProtocols, QObject *parent) :
    QObject(parent),
    d_ptr(new QWebSocketPrivate(this))
{ }

QWebSocket::~QWebSocket()
{ }

QT_END_NAMESPACE
