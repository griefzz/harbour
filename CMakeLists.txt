cmake_minimum_required(VERSION 3.27)

file(STRINGS VERSION CURRENT_VERSION)

project(harbour
    VERSION ${CURRENT_VERSION}
    DESCRIPTION "A Modern C++ WebServer Framework"
    HOMEPAGE_URL "github.com/griefzz/harbour"
    LANGUAGES CXX)

# #############################
# CMake Options
# #############################
option(HARBOUR_BUILD_EXAMPLES "Build the examples in the project" PROJECT_IS_TOP_LEVEL)
option(HARBOUR_BUILD_STANDALONE "Build the standalone harbour executable" PROJECT_IS_TOP_LEVEL)
option(HARBOUR_BUILD_TESTS "Build the harbour test suite" PROJECT_IS_TOP_LEVEL)
option(HARBOUR_BUILD_FUZZ "Build the harbour fuzz testing suite" PROJECT_IS_TOP_LEVEL)
option(HARBOUR_BUILD_BENCHMARKS "Build the harbour benchmark suite" PROJECT_IS_TOP_LEVEL)

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