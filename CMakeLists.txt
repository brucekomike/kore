cmake_minimum_required(VERSION 3.16)

project(kore VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 QUIET COMPONENTS Widgets Test)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets Test)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

add_library(kore_core STATIC
    src/ProjectInfo.h
    src/WorkspaceResolver.h
    src/WorkspaceResolver.cpp
    src/ProjectScanner.h
    src/ProjectScanner.cpp
    src/AppSettings.h
    src/AppSettings.cpp
)

target_include_directories(kore_core PUBLIC src)
target_link_libraries(kore_core PUBLIC Qt${QT_VERSION_MAJOR}::Widgets)

add_executable(kore
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/SettingsDialog.h
    src/SettingsDialog.cpp
)

target_link_libraries(kore PRIVATE kore_core Qt${QT_VERSION_MAJOR}::Widgets)

enable_testing()
add_subdirectory(tests)
