
cmake_minimum_required (VERSION 3.11)
project (Fuzzer)
set(CMAKE_C_COMPILER clang-17)
set(CMAKE_CXX_COMPILER clang-17)
set(CMAKE_CXX_STANDARD 20) # force c++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # disable compiler extensions

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(fuzzer fuzzer.cpp)

target_compile_options(fuzzer
            PRIVATE $<$<C_COMPILER_ID:Clang>:-g -O1 -fsanitize=fuzzer>
            )

target_link_libraries(fuzzer
            PRIVATE $<$<C_COMPILER_ID:Clang>:-fsanitize=fuzzer>
            )
            
target_include_directories(fuzzer
            PUBLIC ../include/
            PRIVATE ../src/)
        
target_sources(fuzzer
            PRIVATE ../src/io.cpp
            PRIVATE ../src/request.cpp
            PRIVATE ../src/response.cpp
            PUBLIC ../include/harbour/forms.hpp
            PUBLIC ../include/harbour/jsonable.hpp
            PUBLIC ../include/harbour/stringable.hpp
            PUBLIC ../include/harbour/logger.hpp
            PUBLIC ../include/harbour/result.hpp
            PUBLIC ../include/harbour/middleware.hpp
            PUBLIC ../include/harbour/handlers.hpp
            PUBLIC ../include/harbour/request.hpp
            PUBLIC ../include/harbour/response.hpp
            PUBLIC ../include/harbour/config.hpp
            PUBLIC ../include/harbour/harbour.hpp)

include(FetchContent)
FetchContent_Declare(
                json
                GIT_REPOSITORY https://github.com/nlohmann/json.git
                GIT_TAG v3.11.3
                GIT_SHALLOW 1
                GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(json)
target_link_libraries(fuzzer PUBLIC nlohmann_json::nlohmann_json)