cache()

QT += core network websockets testlib

TARGET	= unittests
CONFIG  += console
CONFIG	+= c++11
CONFIG  -= app_bundle

TEMPLATE = app

SOURCES += \
        main.cpp \
        tst_websockets.cpp \
        tst_compliance.cpp \
        tst_dataprocessor.cpp

HEADERS += \
    unittests.h
