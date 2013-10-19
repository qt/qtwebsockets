CONFIG += console
CONFIG += c++11
CONFIG += testcase
CONFIG -= app_bundle

TEMPLATE = app

TARGET = tst_dataprocessor

QT = core testlib websockets websockets-private

SOURCES += tst_dataprocessor.cpp

QMAKE_CXXFLAGS_DEBUG += -O0

requires(contains(QT_CONFIG, private_tests))
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
