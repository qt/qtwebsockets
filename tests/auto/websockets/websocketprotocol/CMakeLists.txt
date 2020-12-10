# Generated from websocketprotocol.pro.

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_websocketprotocol Test:
#####################################################################

qt_internal_add_test(tst_websocketprotocol
    SOURCES
        tst_websocketprotocol.cpp
    PUBLIC_LIBRARIES
        Qt::WebSocketsPrivate
)

#### Keys ignored in scope 1:.:.:websocketprotocol.pro:<TRUE>:
# TEMPLATE = "app"
# _REQUIREMENTS = "qtConfig(private_tests)"
