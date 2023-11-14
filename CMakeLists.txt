cmake_minimum_required(VERSION 3.25)

set(PROJECT_NAME "server")

# Reads in our VERSION file and sticks in it CURRENT_VERSION variable
file(STRINGS VERSION CURRENT_VERSION)

# For simplicity, the name of the project is also the name of the target
project(${PROJECT_NAME} VERSION ${CURRENT_VERSION})

# Add our server executable
add_executable("${PROJECT_NAME}" src/main.cpp)

# Enable C++20
target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_20)

if(WIN32)
    # Disable dumb warnings
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    # Link winsock for sockets
    target_link_libraries("${PROJECT_NAME}" wsock32 ws2_32)
endif()
