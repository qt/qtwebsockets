QT       += core websockets
QT       -= gui

TARGET = sslechoclient
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    main.cpp \
    sslechoclient.cpp

HEADERS += \
    sslechoclient.h
