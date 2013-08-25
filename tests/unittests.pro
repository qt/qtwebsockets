cache()

QT += core network

TARGET	= unittests
CONFIG  += console
CONFIG	+= c++11
CONFIG  -= app_bundle

TEMPLATE = app

include(../src/qwebsocket.pri)

# Remove the main.cpp file from the sources.
S = $$SOURCES
SOURCES = \
	tst_compliance.cpp
for(F, S) {
	M = $$find(F, main.cpp)
	count(M, 0) {
		SOURCES += $$F
	}
}

SOURCES += \
		main.cpp \
		tst_websockets.cpp

HEADERS += \
	unittests.h

INCLUDEPATH +=
DEPENDPATH +=

QT += testlib

DEFINES += SRCDIR=\\\"$$PWD/\\\"
