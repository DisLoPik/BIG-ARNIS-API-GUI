cmake_minimum_required(VERSION 3.21)

project(ArnisGlobeBuilder VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core)

qt_standard_project_setup()

qt_add_executable(ArnisGlobeBuilder
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/LandMask.h
    src/LandMask.cpp
)

target_link_libraries(ArnisGlobeBuilder PRIVATE
    Qt6::Widgets
    Qt6::Core
)