# Generated from qmlwebsockets.pro.

#####################################################################
## qmlwebsockets Plugin:
#####################################################################

add_qml_module(qmlwebsockets
    CPP_PLUGIN
    URI "QtWebSockets"
    VERSION "1.0" #special case
    #VERSION "1.${CMAKE_PROJECT_VERSION_MINOR}"
    SOURCES
        qmlwebsockets_plugin.cpp qmlwebsockets_plugin.h
        qqmlwebsocket.cpp qqmlwebsocket.h
        qqmlwebsocketserver.cpp qqmlwebsocketserver.h
    LIBRARIES
        Qt::CorePrivate
        Qt::QmlPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Qml
        Qt::WebSockets
)

#### Keys ignored in scope 1:.:.:qmlwebsockets.pro:<TRUE>:
# IMPORT_VERSION = "1.$$QT_MINOR_VERSION"
# OTHER_FILES = "qmldir"
# TARGETPATH = "QtWebSockets"
# _LOADED = "qml_plugin"
