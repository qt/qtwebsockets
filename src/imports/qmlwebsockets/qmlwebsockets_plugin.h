#ifndef QMLWEBSOCKET_PLUGIN_H
#define QMLWEBSOCKET_PLUGIN_H

#include <QQmlExtensionPlugin>

#include "qqmlwebsocket.h"

QT_BEGIN_NAMESPACE

class QmlWebsocket_plugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri);
};

QT_END_NAMESPACE

#endif // QMLWEBSOCKET_PLUGIN_H
