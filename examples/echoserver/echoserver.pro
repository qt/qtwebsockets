QT       += core websockets
QT       -= gui

TARGET = echoserver
CONFIG   += console
CONFIG   -= app_bundle

mac:QMAKE_CXXFLAGS += -Wall -Werror -Wextra

TEMPLATE = app

SOURCES += \
    main.cpp \
    echoserver.cpp

HEADERS += \
    echoserver.h
