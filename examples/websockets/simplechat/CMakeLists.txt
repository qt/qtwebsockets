# Generated from simplechat.pro.

cmake_minimum_required(VERSION 3.14)
project(chatserver LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/websockets/simplechat")

find_package(Qt6 COMPONENTS WebSockets)

qt_add_executable(chatserver
    chatserver.cpp chatserver.h
    main.cpp
)
target_link_libraries(chatserver PUBLIC
    Qt::WebSockets
)

install(TARGETS chatserver
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
