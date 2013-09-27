#include "qmlwebsockets_plugin.h"

#include <QtQml>

void QmlWebsocket_plugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("Qt.Playground.WebSockets"));

    int major = 1;
    int minor = 0;

    // @uri Qt.Playground.WebSockets

    qmlRegisterType<QQmlWebSocket>(uri, major, minor, "WebSocket");
}
