load(qt_build_config)
TARGET = QtWebSockets

QT = core network

TEMPLATE = lib

DEFINES += QTWEBSOCKETS_LIBRARY

QMAKE_DOCS = $$PWD/doc/qtwebsockets.qdocconf
OTHER_FILES += doc/src/*.qdoc   # show .qdoc files in Qt Creator
OTHER_FILES += doc/snippets/*.cpp
OTHER_FILES += doc/qtwebsockets.qdocconf

PUBLIC_HEADERS += \
    $$PWD/qwebsockets_global.h \
    $$PWD/qwebsocket.h \
    $$PWD/qwebsocketserver.h \
    $$PWD/qwebsocketprotocol.h \
    $$PWD/qwebsocketcorsauthenticator.h

PRIVATE_HEADERS += \
    $$PWD/qwebsocket_p.h \
    $$PWD/qwebsocketserver_p.h \
    $$PWD/qwebsocketprotocol_p.h \
    $$PWD/qwebsockethandshakerequest_p.h \
    $$PWD/qwebsockethandshakeresponse_p.h \
    $$PWD/qwebsocketdataprocessor_p.h \
    $$PWD/qwebsocketcorsauthenticator_p.h \
    $$PWD/qwebsocketframe_p.h

SOURCES += \
    $$PWD/qwebsocket.cpp \
    $$PWD/qwebsocket_p.cpp \
    $$PWD/qwebsocketserver.cpp \
    $$PWD/qwebsocketserver_p.cpp \
    $$PWD/qwebsocketprotocol.cpp \
    $$PWD/qwebsockethandshakerequest_p.cpp \
    $$PWD/qwebsockethandshakeresponse_p.cpp \
    $$PWD/qwebsocketdataprocessor_p.cpp \
    $$PWD/qwebsocketcorsauthenticator.cpp \
    $$PWD/qwebsocketframe_p.cpp

contains(QT_CONFIG, openssl) | contains(QT_CONFIG, openssl-linked) {
    SOURCES += $$PWD/qsslserver_p.cpp
    PRIVATE_HEADERS += $$PWD/qsslserver_p.h
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

load(qt_module)
