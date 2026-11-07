cmake_minimum_required(VERSION 3.21)

project(QtTelemetryMonitor
    VERSION 0.1.0
    DESCRIPTION "Real-time UDP telemetry monitor built with Qt 6"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.11 REQUIRED COMPONENTS Core Network Quick Graphs Test)
qt_standard_project_setup(REQUIRES 6.11)

# Put every QML module under one import path so tooling (qmllint, Qt Creator) finds them.
set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)

enable_testing()

add_subdirectory(common)
add_subdirectory(simulator)
add_subdirectory(monitor)
add_subdirectory(tests)
