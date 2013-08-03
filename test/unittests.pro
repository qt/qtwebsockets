#-------------------------------------------------
#
# Project created by QtCreator 2011-06-13T16:50:36
#
#-------------------------------------------------

# Determine the platform: if using a cross-compiler -> add it to the config flags.
!contains(QMAKE_CXX, g++) {
		CONFIG += embedded
}

QT += core network
CONFIG += c++11

embedded { # Vanilla Qt libraries are differently deployed then the dev libraries of Debian.
		QT += multimedia
}
CONFIG += mobility
MOBILITY = multimedia

unix:!macx {
		QT += dbus
}


include(../source/websocket.pri)

#include(../source/buildsystem/buildsystem.pri)

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

TARGET = unittests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += SRCDIR=\\\"$$PWD/\\\"
