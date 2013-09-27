QT += network

TARGET = QtWebSockets

TEMPLATE += lib

DEFINES += QTWEBSOCKETS_LIBRARY

load(qt_module)

QMAKE_DOCS = $$PWD/doc/qwebsockets.qdocconfig

PUBLIC_HEADERS += \
    $$PWD/qwebsocket.h \
    $$PWD/qwebsocketserver.h \
    $$PWD/qwebsocketprotocol.h \
    $$PWD/qwebsocketsglobal.h \
    $$PWD/qcorsauthenticator.h

PRIVATE_HEADERS += \
    $$PWD/qwebsocket_p.h \
    $$PWD/qwebsocketserver_p.h \
    $$PWD/handshakerequest_p.h \
    $$PWD/handshakeresponse_p.h \
    $$PWD/dataprocessor_p.h \
    $$PWD/qcorsauthenticator_p.h

SOURCES += \
    $$PWD/qwebsocket.cpp \
    $$PWD/qwebsocket_p.cpp \
    $$PWD/qwebsocketserver.cpp \
    $$PWD/qwebsocketserver_p.cpp \
    $$PWD/qwebsocketprotocol.cpp \
    $$PWD/handshakerequest_p.cpp \
    $$PWD/handshakeresponse_p.cpp \
    $$PWD/dataprocessor_p.cpp \
    $$PWD/qcorsauthenticator.cpp

#mac:QMAKE_FRAMEWORK_BUNDLE_NAME = $$TARGET
#mac:QMAKE_CXXFLAGS += -Wall -Werror -Wextra



