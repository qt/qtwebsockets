# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from websockets.pro.

add_subdirectory(qwebsocketcorsauthenticator)
add_subdirectory(qwebsocket)
add_subdirectory(qwebsocketserver)
if(QT_FEATURE_private_tests)
    add_subdirectory(websocketprotocol)
    add_subdirectory(dataprocessor)
    add_subdirectory(websocketframe)
    add_subdirectory(handshakerequest)
    add_subdirectory(handshakeresponse)
    add_subdirectory(qdefaultmaskgenerator)
endif()
