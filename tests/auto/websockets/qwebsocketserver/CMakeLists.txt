# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qwebsocketserver.pro.

#####################################################################
## tst_qwebsocketserver Test:
#####################################################################

qt_internal_add_test(tst_qwebsocketserver
    SOURCES
        tst_qwebsocketserver.cpp
    LIBRARIES
        Qt::WebSockets
)

# Resources:
set(qwebsocketshared_resource_files
    "../shared/localhost.cert"
    "../shared/localhost.key"
)

qt_internal_add_resource(tst_qwebsocketserver "qwebsocketshared"
    PREFIX
        "/"
    BASE
        "../shared"
    FILES
        ${qwebsocketshared_resource_files}
)


#### Keys ignored in scope 1:.:.:qwebsocketserver.pro:<TRUE>:
# TEMPLATE = "app"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qwebsocketserver CONDITION boot2qt
    DEFINES
        SHOULD_CHECK_SYSCALL_SUPPORT
)
