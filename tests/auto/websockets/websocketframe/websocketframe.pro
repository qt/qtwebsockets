CONFIG += console
CONFIG += testcase
CONFIG -= app_bundle

TEMPLATE = app

TARGET = tst_websocketframe

QT = core testlib websockets websockets-private

SOURCES += tst_websocketframe.cpp

requires(contains(QT_CONFIG, private_tests))
