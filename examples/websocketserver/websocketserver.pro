#-------------------------------------------------
#
# Project created by QtCreator 2013-08-03T19:54:35
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = websocketserver
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

include(../../source/websocket.pri)

SOURCES += main.cpp \
    helloworldserver.cpp

HEADERS += \
    helloworldserver.h
