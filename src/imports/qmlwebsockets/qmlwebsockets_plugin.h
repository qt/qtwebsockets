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
****************************************************************************/

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
    void registerTypes(const char *uri);
};

QT_END_NAMESPACE

#endif // QMLWEBSOCKET_PLUGIN_H
