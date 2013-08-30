cache()

QT += core network

TARGET	= unittests
CONFIG  += console
CONFIG	+= c++11
CONFIG  -= app_bundle

TEMPLATE = app

mac:QMAKE_CXXFLAGS += -Wall -Werror -Wextra

include(../src/qwebsockets.pri)

# Remove the main.cpp file from the sources.
#S = $$SOURCES
#for(F, S) {
#    M = $$find(F, main.cpp)
#    count(M, 0) {
#        SOURCES += $$F
#    }
#}

SOURCES += \
        main.cpp \
        tst_websockets.cpp \
        tst_compliance.cpp \
        tst_dataprocessor.cpp

HEADERS += \
    unittests.h

INCLUDEPATH +=
DEPENDPATH +=

QT += testlib

DEFINES += SRCDIR=\\\"$$PWD/\\\"
