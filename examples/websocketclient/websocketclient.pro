QT       += core
QT       -= gui

TARGET = websocketclient
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

include(../../source/websocket.pri)

SOURCES += main.cpp \
	websocketclient.cpp

HEADERS += \
	websocketclient.h
