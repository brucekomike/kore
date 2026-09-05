cmake_minimum_required(VERSION 3.16)

project(kore VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)
set(QT_VERSION_MAJOR 6)

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
    src/resources.qrc
)

if(APPLE)
    set_target_properties(kore PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "kore"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.kore.workspace"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()

target_link_libraries(kore PRIVATE kore_core Qt${QT_VERSION_MAJOR}::Widgets)

include(GNUInstallDirs)
install(TARGETS kore
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION .
)

if(UNIX AND NOT APPLE)
    install(FILES packaging/kore.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
    install(FILES icon.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        RENAME kore.svg
    )
endif()

enable_testing()
add_subdirectory(tests)
