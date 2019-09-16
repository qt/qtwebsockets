# Generated from qmlwebsocketclient.pro.

cmake_minimum_required(VERSION 3.14)
project(qmlwebsocketclient LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS WebSockets)

add_qt_gui_executable(qmlwebsocketclient
    main.cpp
)
target_link_libraries(qmlwebsocketclient PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::WebSockets
)

# Resources:
set(data_resource_files
    "qml/qmlwebsocketclient/main.qml"
)

QT6_ADD_RESOURCES(qmlwebsocketclient "data"
    PREFIX
        "/"
    FILES
        ${data_resource_files}
)


install(TARGETS qmlwebsocketclient
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
