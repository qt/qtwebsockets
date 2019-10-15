# Generated from handshakeresponse.pro.

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_handshakeresponse Test:
#####################################################################

add_qt_test(tst_handshakeresponse
    SOURCES
        tst_handshakeresponse.cpp
    LIBRARIES
        Qt::WebSocketsPrivate
    PUBLIC_LIBRARIES
        Qt::WebSockets
)

#### Keys ignored in scope 1:.:.:handshakeresponse.pro:<TRUE>:
# TEMPLATE = "app"
# _REQUIREMENTS = "qtConfig(private_tests)"
