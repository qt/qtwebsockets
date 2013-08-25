QT       += core
QT       -= gui

TARGET = echoserver
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

include(../../src/qwebsockets.pri)

SOURCES += \
	main.cpp \
	echoserver.cpp

HEADERS += \
	echoserver.h
