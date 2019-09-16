# Generated from websockets.pro.

add_subdirectory(echoclient)
add_subdirectory(echoserver)
add_subdirectory(simplechat)
if(TARGET Qt::Quick)
    add_subdirectory(qmlwebsocketclient)
    add_subdirectory(qmlwebsocketserver)
endif()
if(QT_FEATURE_ssl)
    add_subdirectory(sslechoserver)
    add_subdirectory(sslechoclient)
endif()
