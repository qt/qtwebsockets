# Generated from sslechoclient.pro.

cmake_minimum_required(VERSION 3.14)
project(sslechoclient LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/websockets/sslechoclient")

find_package(Qt6 COMPONENTS WebSockets)

add_executable(sslechoclient
    main.cpp
    sslechoclient.cpp sslechoclient.h
)
target_link_libraries(sslechoclient PUBLIC
    Qt::WebSockets
)

install(TARGETS sslechoclient
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
