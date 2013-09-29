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
#ifndef QWEBSOCKETCORSAUTHENTICATOR_H
#define QWEBSOCKETCORSAUTHENTICATOR_H

#include "qwebsockets_global.h"

QT_BEGIN_NAMESPACE

class QWebSocketCorsAuthenticatorPrivate;

class Q_WEBSOCKETS_EXPORT QWebSocketCorsAuthenticator
{
    Q_DECLARE_PRIVATE(QWebSocketCorsAuthenticator)

public:
    QWebSocketCorsAuthenticator(const QString &origin);
    ~QWebSocketCorsAuthenticator();
    QWebSocketCorsAuthenticator(const QWebSocketCorsAuthenticator &other);

    QWebSocketCorsAuthenticator &operator =(const QWebSocketCorsAuthenticator &other);

    QString origin() const;

    void setAllowed(bool allowed);
    bool allowed() const;

private:
    QWebSocketCorsAuthenticatorPrivate * const d_ptr;
};

#endif // QWEBSOCKETCORSAUTHENTICATOR_H
