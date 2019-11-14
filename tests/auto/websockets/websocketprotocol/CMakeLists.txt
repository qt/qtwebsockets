# Generated from websocketprotocol.pro.

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_websocketprotocol Test:
#####################################################################

qt_add_test(tst_websocketprotocol
    SOURCES
        tst_websocketprotocol.cpp
    LIBRARIES
        Qt::WebSocketsPrivate
    PUBLIC_LIBRARIES
        Qt::WebSockets
)

#### Keys ignored in scope 1:.:.:websocketprotocol.pro:<TRUE>:
# TEMPLATE = "app"
# _REQUIREMENTS = "qtConfig(private_tests)"
