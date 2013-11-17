QT = core websockets qml

TARGETPATH = Qt/WebSockets

HEADERS +=  qmlwebsockets_plugin.h \
            qqmlwebsocket.h

SOURCES +=  qmlwebsockets_plugin.cpp \
            qqmlwebsocket.cpp

OTHER_FILES += qmldir

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

load(qml_plugin)
