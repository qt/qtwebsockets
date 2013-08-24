QT       += core
QT       -= gui

TARGET = echoclient
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

include(../../src/websocket.pri)

SOURCES += main.cpp \
	echoclient.cpp

HEADERS += \
	echoclient.h
