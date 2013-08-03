#-------------------------------------------------
#
# Project created by QtCreator 2012-10-27T16:08:13
#
#-------------------------------------------------

QT       *= network

#include(externals/qtwebsocket/qtwebsocket.pri)

SOURCES += $$PWD/websocket.cpp \
	$$PWD/websocketserver.cpp \
	#$$PWD/websocketserver_p.cpp \
	$$PWD/websocketprotocol.cpp \
	$$PWD/handshakerequest.cpp \
	$$PWD/handshakeresponse.cpp \
	#$$PWD/websocketrequest.cpp \
	$$PWD/dataprocessor.cpp

HEADERS += $$PWD/websocket.h \
	$$PWD/websocketserver.h \
	#$$PWD/websocketserver_p.h \
	$$PWD/websocketprotocol.h \
	$$PWD/handshakerequest.h \
	$$PWD/handshakeresponse.h \
	#$$PWD/websocketrequest.h \
	$$PWD/dataprocessor.h

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
