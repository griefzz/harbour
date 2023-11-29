cmake_minimum_required(VERSION 3.25)

# Reads in our VERSION file and sticks in it CURRENT_VERSION variable
file(STRINGS VERSION CURRENT_VERSION)

# For simplicity, the name of the project is also the name of the target
set(PROJECT_NAME "harbour")

project(${PROJECT_NAME} 
        VERSION ${CURRENT_VERSION}
        DESCRIPTION  "C++ Webserver and personal Website"
        HOMEPAGE_URL "github.com/griefzz/harbour"
        LANGUAGES    CXX)

set(CMAKE_CXX_STANDARD          20) # force c++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF) # disable compiler extensions

# Add our server executable
add_executable("${PROJECT_NAME}")

target_include_directories("${PROJECT_NAME}"
                           PUBLIC  include/
                           PRIVATE src/)

target_sources("${PROJECT_NAME}"
               PRIVATE src/main.cpp
               PRIVATE src/io.cpp
               PUBLIC include/harbour/forms.hpp
               PUBLIC include/harbour/jsonable.hpp
               PUBLIC include/harbour/stringable.hpp
               PUBLIC include/harbour/logger.hpp
               PUBLIC include/harbour/result.hpp
               PUBLIC include/harbour/middleware.hpp
               PUBLIC include/harbour/handlers.hpp)

if(WIN32)
    # Disable dumb warnings
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    # Link winsock for sockets
    target_link_libraries("${PROJECT_NAME}" wsock32 ws2_32)
endif()

# Include brotli compression when not on windows
if (NOT WIN32)
    include(FetchContent)
    FetchContent_Declare(
        Brotli
        GIT_REPOSITORY https://github.com/google/brotli
        GIT_TAG master
    )
    FetchContent_MakeAvailable(Brotli)

    find_package(OpenSSL REQUIRED)

    if(OPENSSL_FOUND)
        include_directories(${OPENSSL_INCLUDE_DIR})
        target_link_libraries("${PROJECT_NAME}" ${OPENSSL_LIBRARIES})
    endif()

    target_link_libraries("${PROJECT_NAME}" ${BROTLI_LIBRARIES})
endif()