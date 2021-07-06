qt_internal_add_qml_module(qmlwebsockets
    URI "QtWebSockets"
    VERSION "${PROJECT_VERSION}"
    PAST_MAJOR_VERSIONS 1
    CLASS_NAME QtWebSocketsDeclarativeModule
    PLUGIN_TARGET qmlwebsockets
    NO_PLUGIN_OPTIONAL
    NO_GENERATE_PLUGIN_SOURCE
    NO_GENERATE_QMLTYPES
    INSTALL_SOURCE_QMLTYPES "plugins.qmltypes"
    SOURCES
        qmlwebsockets_plugin.cpp qmlwebsockets_plugin.h
        qqmlwebsocket.cpp qqmlwebsocket.h
        qqmlwebsocketserver.cpp qqmlwebsocketserver.h
    LIBRARIES
        Qt::CorePrivate
        Qt::QmlPrivate
        Qt::WebSockets
)
