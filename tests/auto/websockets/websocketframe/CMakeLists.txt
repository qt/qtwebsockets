# Generated from websocketframe.pro.

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_websocketframe Test:
#####################################################################

add_qt_test(tst_websocketframe
    SOURCES
        tst_websocketframe.cpp
    LIBRARIES
        Qt::WebSocketsPrivate
    PUBLIC_LIBRARIES
        Qt::WebSockets
)

#### Keys ignored in scope 1:.:.:websocketframe.pro:<TRUE>:
# TEMPLATE = "app"
# _REQUIREMENTS = "qtConfig(private_tests)"
