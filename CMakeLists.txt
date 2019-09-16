cmake_minimum_required(VERSION 3.15.0)

project(QtWebSockets
    VERSION 6.0.0
    DESCRIPTION "Qt WebSockets Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Network)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Quick QuickTest)
qt_build_repo()
