QT       += core
QT       -= gui

TARGET = echoclient
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

include(../../src/qwebsocket.pri)

SOURCES += \
	main.cpp \
	echoclient.cpp

HEADERS += \
	echoclient.h
