cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "server")

# Reads in our VERSION file and sticks in it CURRENT_VERSION variable
# Be sure the file has no newlines!
file(STRINGS VERSION CURRENT_VERSION)

# For simplicity, the name of the project is also the name of the target
project(${PROJECT_NAME} VERSION ${CURRENT_VERSION})

# Set enablements
set(SERVER_ENABLE_BENCHMARKS FALSE)
set(SERVER_ENABLE_TESTS FALSE)

# Add our server executable
add_executable("${PROJECT_NAME}" src/main.cpp)

# C++23, please
target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_23)

if(WIN32)
    target_link_libraries("${PROJECT_NAME}" wsock32 ws2_32)
endif()

# Optionally include our tests
if(SERVER_ENABLE_TESTS)
    if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        include(CTest)
    endif()

    if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()