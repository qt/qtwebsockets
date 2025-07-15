// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

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
