// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMLWEBSOCKET_PLUGIN_H
#define QMLWEBSOCKET_PLUGIN_H

#include <QQmlExtensionPlugin>

QT_BEGIN_NAMESPACE

class QtWebSocketsDeclarativeModule : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QtWebSocketsDeclarativeModule(QObject *parent = 0) : QQmlExtensionPlugin(parent) { }
    void registerTypes(const char *uri) override;
};

QT_END_NAMESPACE

#endif // QMLWEBSOCKET_PLUGIN_H
