QT += websockets qml

TARGETPATH = Qt/Playground/WebSockets

HEADERS += qmlwebsockets_plugin.h \
    qqmlwebsocket.h

SOURCES += qmlwebsockets_plugin.cpp \
    qqmlwebsocket.cpp

load(qml_plugin)
