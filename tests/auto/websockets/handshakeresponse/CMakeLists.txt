# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_handshakeresponse LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

# Generated from handshakeresponse.pro.

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_handshakeresponse Test:
#####################################################################

qt_internal_add_test(tst_handshakeresponse
    SOURCES
        tst_handshakeresponse.cpp
    LIBRARIES
        Qt::NetworkPrivate
        Qt::WebSocketsPrivate
)

#### Keys ignored in scope 1:.:.:handshakeresponse.pro:<TRUE>:
# TEMPLATE = "app"
# _REQUIREMENTS = "qtConfig(private_tests)"
