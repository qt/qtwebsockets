QT       += core websockets
QT       -= gui

TARGET = echoclient
CONFIG   += console
CONFIG   -= app_bundle

mac:QMAKE_CXXFLAGS += -Wall -Werror -Wextra

TEMPLATE = app

SOURCES += \
    main.cpp \
    echoclient.cpp

HEADERS += \
    echoclient.h
