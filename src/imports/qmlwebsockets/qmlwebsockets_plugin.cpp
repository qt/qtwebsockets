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

#include "qmlwebsockets_plugin.h"

#include <QtQml>

#include "qqmlwebsocket.h"
#include "qqmlwebsocketserver.h"

QT_BEGIN_NAMESPACE

void QtWebSocketsDeclarativeModule::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("QtWebSockets"));

    // @uri QtWebSockets
    qmlRegisterType<QQmlWebSocket>(uri, 1 /*major*/, 0 /*minor*/, "WebSocket");
    qmlRegisterType<QQmlWebSocket, 1>(uri, 1 /*major*/, 1 /*minor*/, "WebSocket");
    qmlRegisterType<QQmlWebSocketServer>(uri, 1 /*major*/, 0 /*minor*/, "WebSocketServer");

    // Auto-increment the import to stay in sync with ALL future QtQuick minor versions
    qmlRegisterModule(uri, 1, QT_VERSION_MINOR);
}

QT_END_NAMESPACE
