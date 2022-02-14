cmake_minimum_required(VERSION 3.16)
project(echoserver LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/websockets/echoserver")

find_package(Qt6 REQUIRED COMPONENTS WebSockets)

qt_add_executable(echoserver
    echoserver.cpp echoserver.h
    main.cpp
)

set_target_properties(echoserver PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)

target_link_libraries(echoserver PUBLIC
    Qt::WebSockets
)

install(TARGETS echoserver
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
