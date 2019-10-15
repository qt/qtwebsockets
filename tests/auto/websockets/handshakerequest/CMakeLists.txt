# Generated from handshakerequest.pro.

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_handshakerequest Test:
#####################################################################

add_qt_test(tst_handshakerequest
    SOURCES
        tst_handshakerequest.cpp
    LIBRARIES
        Qt::WebSocketsPrivate
    PUBLIC_LIBRARIES
        Qt::WebSockets
)

#### Keys ignored in scope 1:.:.:handshakerequest.pro:<TRUE>:
# TEMPLATE = "app"
# _REQUIREMENTS = "qtConfig(private_tests)"
