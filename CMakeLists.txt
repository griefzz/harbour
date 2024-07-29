cmake_minimum_required(VERSION 3.27)

file(STRINGS VERSION CURRENT_VERSION)

project(harbour
    VERSION ${CURRENT_VERSION}
    DESCRIPTION "A Modern C++ WebServer Framework"
    HOMEPAGE_URL "github.com/griefzz/harbour"
    LANGUAGES CXX)

# Check if Harbour is the main project
set(HARBOUR_IS_MAIN_PROJECT OFF)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(HARBOUR_IS_MAIN_PROJECT ON)
endif()

# #############################
# CMake Options
# #############################
option(HARBOUR_BUILD_EXAMPLES "Build the examples in the project" ${HARBOUR_IS_MAIN_PROJECT})
option(HARBOUR_BUILD_STANDALONE "Build the standalone harbour executable" ${HARBOUR_IS_MAIN_PROJECT})
option(HARBOUR_BUILD_TESTS "Build the harbour test suite" ${HARBOUR_IS_MAIN_PROJECT})
option(HARBOUR_BUILD_FUZZ "Build the harbour fuzz testing suite" ${HARBOUR_IS_MAIN_PROJECT})
option(HARBOUR_BUILD_BENCHMARKS "Build the harbour benchmark suite" ${HARBOUR_IS_MAIN_PROJECT})
option(HARBOUR_SKIP_AUTOMATE_VCPKG "Use local vcpkg installation instead of automate-vcpkg.cmake" OFF)

# #############################
# Harbour Library
# #############################
set(HARBOUR_SOURCES
    "include/harbour/harbour.hpp"
)
add_library(harbour INTERFACE ${HARBOUR_SOURCES})
add_library(harbour::harbour ALIAS harbour)
target_compile_features(harbour INTERFACE cxx_std_20)
target_include_directories(harbour INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# #############################
# Harbour Dependencies
# #############################
include(cmake/fmtlib.cmake)
include(cmake/json.cmake)
include(cmake/unordered_dense.cmake)
include(cmake/llhttp.cmake)
include(cmake/openssl.cmake)
include(cmake/asio.cmake)

# #############################
# Harbour Examples
# #############################
if(HARBOUR_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# #############################
# Harbour Standalone
# #############################
if(HARBOUR_BUILD_STANDALONE)
    add_subdirectory(standalone)
endif()

# #############################
# Harbour Tests
# #############################
if(HARBOUR_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

# #############################
# Harbour Benchmarks
# #############################
if(HARBOUR_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# #############################
# Harbour Fuzzing
# #############################
if(HARBOUR_BUILD_FUZZ)
    add_subdirectory(fuzz)
endif()