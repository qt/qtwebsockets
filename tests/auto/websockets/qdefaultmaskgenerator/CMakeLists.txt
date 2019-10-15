# Generated from qdefaultmaskgenerator.pro.

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_defaultmaskgenerator Test:
#####################################################################

add_qt_test(tst_defaultmaskgenerator
    SOURCES
        tst_defaultmaskgenerator.cpp
    LIBRARIES
        Qt::WebSocketsPrivate
    PUBLIC_LIBRARIES
        Qt::WebSockets
)

#### Keys ignored in scope 1:.:.:qdefaultmaskgenerator.pro:<TRUE>:
# TEMPLATE = "app"
# _REQUIREMENTS = "qtConfig(private_tests)"
