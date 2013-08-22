QT       *= network

SOURCES += $$PWD/websocket.cpp \
	$$PWD/websocketserver.cpp \
	$$PWD/websocketprotocol.cpp \
	$$PWD/handshakerequest_p.cpp \
	$$PWD/handshakeresponse_p.cpp \
	$$PWD/dataprocessor_p.cpp

HEADERS += $$PWD/websocket.h \
	$$PWD/websocketserver.h \
	$$PWD/websocketprotocol.h \
	$$PWD/handshakerequest_p.h \
	$$PWD/handshakeresponse_p.h \
	$$PWD/dataprocessor_p.h

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
