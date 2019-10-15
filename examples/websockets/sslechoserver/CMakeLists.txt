# Generated from sslechoserver.pro.

cmake_minimum_required(VERSION 3.14)
project(sslechoserver LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/websockets/sslechoserver")

find_package(Qt6 COMPONENTS WebSockets)

add_executable(sslechoserver
    main.cpp
    sslechoserver.cpp sslechoserver.h
)
target_link_libraries(sslechoserver PUBLIC
    Qt::WebSockets
)


# Resources:
set(securesocketclient_resource_files
    "localhost.cert"
    "localhost.key"
)

qt6_add_resources(sslechoserver "securesocketclient"
    PREFIX
        "/"
    FILES
        ${securesocketclient_resource_files}
)

install(TARGETS sslechoserver
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
