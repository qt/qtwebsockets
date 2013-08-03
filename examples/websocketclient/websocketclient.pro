#-------------------------------------------------
#
# Project created by QtCreator 2013-08-03T19:04:16
#
#-------------------------------------------------

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
