cmake_minimum_required(VERSION 3.16)
project(focusflowd)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find required packages
find_package(Qt6 COMPONENTS Core Gui Widgets DBus REQUIRED)

# Find nlohmann_json
find_package(nlohmann_json REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/action_config_window.cpp
    src/action_config_window.h
    src/dbus_listener.cpp
    src/dbus_listener.h
    src/action_manager.cpp
    src/action_manager.h
    src/resources.qrc
)

# Create executable
add_executable(focusflowd ${SOURCES})

# Link libraries
target_link_libraries(focusflowd
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    nlohmann_json::nlohmann_json
    stdc++fs
)

# Installation
install(TARGETS focusflowd DESTINATION bin)
install(FILES focusflowd.desktop DESTINATION share/applications)
install(FILES icons/hicolor/scalable/apps/focusflowd.svg DESTINATION share/icons/hicolor/scalable/apps)
