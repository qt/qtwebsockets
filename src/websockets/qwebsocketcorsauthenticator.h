/****************************************************************************
**
** Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/
#ifndef QWEBSOCKETCORSAUTHENTICATOR_H
#define QWEBSOCKETCORSAUTHENTICATOR_H

#include "QtWebSockets/qwebsockets_global.h"
#include <memory>

QT_BEGIN_NAMESPACE

class QWebSocketCorsAuthenticatorPrivate;

class Q_WEBSOCKETS_EXPORT QWebSocketCorsAuthenticator
{
    Q_DECLARE_PRIVATE(QWebSocketCorsAuthenticator)

public:
    explicit QWebSocketCorsAuthenticator(const QString &origin);
    ~QWebSocketCorsAuthenticator();
    explicit QWebSocketCorsAuthenticator(const QWebSocketCorsAuthenticator &other);

    QWebSocketCorsAuthenticator(QWebSocketCorsAuthenticator &&other) noexcept;
    QWebSocketCorsAuthenticator &operator =(QWebSocketCorsAuthenticator &&other) noexcept;

    void swap(QWebSocketCorsAuthenticator &other) noexcept;

    QWebSocketCorsAuthenticator &operator =(const QWebSocketCorsAuthenticator &other);

    QString origin() const;

    void setAllowed(bool allowed);
    bool allowed() const;

private:
    std::unique_ptr<QWebSocketCorsAuthenticatorPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBSOCKETCORSAUTHENTICATOR_H
