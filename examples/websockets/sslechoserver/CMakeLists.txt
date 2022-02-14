cmake_minimum_required(VERSION 3.16)
project(sslechoserver LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/websockets/sslechoserver")

find_package(Qt6 REQUIRED COMPONENTS WebSockets)

qt_add_executable(sslechoserver
    main.cpp
    sslechoserver.cpp sslechoserver.h
)

set_target_properties(sslechoserver PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
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
