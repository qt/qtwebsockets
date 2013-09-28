QT += network

TARGET = QtWebSockets2

TEMPLATE += lib

load(qt_module)

PUBLIC_HEADERS += \
    qwebsockets_global.h \
    qwebsocket.h


PRIVATE_HEADERS += \
    qwebsocket_p.h

SOURCES += \
    qwebsocket.cpp \
    qwebsocket_p.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
