# Generated from qwebsocketserver.pro.

#####################################################################
## tst_qwebsocketserver Test:
#####################################################################

add_qt_test(tst_qwebsocketserver
    SOURCES
        tst_qwebsocketserver.cpp
    PUBLIC_LIBRARIES
        Qt::WebSockets
)

# Resources:
set_source_files_properties("../shared/localhost.cert"
    PROPERTIES QT_RESOURCE_ALIAS "localhost.cert"
)
set_source_files_properties("../shared/localhost.key"
    PROPERTIES QT_RESOURCE_ALIAS "localhost.key"
)
set(qwebsocketshared_resource_files
    "localhost.cert"
    "localhost.key"
)

add_qt_resource(tst_qwebsocketserver "qwebsocketshared"
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

extend_target(tst_qwebsocketserver CONDITION boot2qt
    DEFINES
        SHOULD_CHECK_SYSCALL_SUPPORT
)
