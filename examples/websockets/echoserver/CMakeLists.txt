# Generated from echoserver.pro.

cmake_minimum_required(VERSION 3.14)
project(echoserver LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples")

find_package(Qt6 COMPONENTS WebSockets)

add_executable(echoserver
    echoserver.cpp echoserver.h
    main.cpp
)
target_link_libraries(echoserver PUBLIC
    Qt::WebSockets
)

install(TARGETS echoserver
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
