CONFIG += console
CONFIG += c++11
CONFIG += testcase
CONFIG -= app_bundle

TEMPLATE = app

TARGET = tst_websocketcorsauthenticator

QT = core testlib websockets websockets-private

SOURCES += tst_websocketcorsauthenticator.cpp

requires(contains(QT_CONFIG, private_tests))
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
