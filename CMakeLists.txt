cmake_minimum_required(VERSION 3.16)
project(focusflowd)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find required packages
find_package(Qt6 COMPONENTS Core DBus REQUIRED)

# Find nlohmann_json
find_package(nlohmann_json REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/dbus_listener.cpp
    src/dbus_listener.h
    src/action_manager.cpp
    src/action_manager.h
)

# Create executable
add_executable(focusflowd ${SOURCES})

# Link libraries
target_link_libraries(focusflowd
    Qt6::Core
    Qt6::DBus
    nlohmann_json::nlohmann_json
    stdc++fs
)

option(SYSTEMD
    "Install systemd user service file"
    ON
)

option(OPENRC
    "Install OpenRC user service file"
    OFF
)

if(OPENRC AND SYSTEMD)
    message(STATUS "Both OPENRC and SYSTEMD are enabled; disabling SYSTEMD and keeping OPENRC")
    set(SYSTEMD OFF CACHE BOOL "Install systemd user service file" FORCE)
elseif(OPENRC)
    set(SYSTEMD OFF CACHE BOOL "Install systemd user service file" FORCE)
elseif(SYSTEMD)
    set(OPENRC OFF CACHE BOOL "Install OpenRC user service file" FORCE)
endif()

# Installation
install(TARGETS focusflowd DESTINATION bin)

# Install systemd user service
if(SYSTEMD)
    configure_file(
        focusflowd.service
        ${CMAKE_CURRENT_BINARY_DIR}/focusflowd.service
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/focusflowd.service
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/systemd/user
    )
endif()

# Install OpenRC user service
if(OPENRC)
    configure_file(
        focusflowd.in
        ${CMAKE_CURRENT_BINARY_DIR}/focusflowd.openrc
        @ONLY
    )
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/focusflowd.openrc
        RENAME focusflowd DESTINATION etc/init.d
    )
endif()
