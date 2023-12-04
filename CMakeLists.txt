cmake_minimum_required(VERSION 3.25)

# Reads in our VERSION file and sticks in it CURRENT_VERSION variable
file(STRINGS VERSION CURRENT_VERSION)

# For simplicity, the name of the project is also the name of the target
set(PROJECT_NAME "harbour")

project(${PROJECT_NAME} 
        VERSION ${CURRENT_VERSION}
        DESCRIPTION  "C++ webserver and personal website"
        HOMEPAGE_URL "github.com/griefzz/harbour"
        LANGUAGES    CXX)
        
set(CMAKE_CXX_STANDARD          20) # force c++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF) # disable compiler extensions
#set(CMAKE_CXX_CLANG_TIDY "clang-tidy-17")
Set(FETCHCONTENT_QUIET FALSE)

# Use Brotli compression
set(ENABLE_COMPRESSION true)

# Use TLS
set(ENABLE_TLS true)

# Add our server executable
add_executable("${PROJECT_NAME}")

target_include_directories("${PROJECT_NAME}"
                           PUBLIC  include/
                           PRIVATE src/)

target_sources("${PROJECT_NAME}"
               PRIVATE src/main.cpp
               PRIVATE src/io.cpp
               PRIVATE src/request.cpp
               PRIVATE src/response.cpp
               PUBLIC include/harbour/forms.hpp
               PUBLIC include/harbour/jsonable.hpp
               PUBLIC include/harbour/stringable.hpp
               PUBLIC include/harbour/logger.hpp
               PUBLIC include/harbour/result.hpp
               PUBLIC include/harbour/middleware.hpp
               PUBLIC include/harbour/handlers.hpp
               PUBLIC include/harbour/request.hpp
               PUBLIC include/harbour/response.hpp
               PUBLIC include/harbour/config.hpp
               PUBLIC include/harbour/harbour.hpp)

if(WIN32)
    # Disable dumb warnings
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DHARBOUR_ENABLE_COMPRESSION=false)

    # Link winsock for sockets
    target_link_libraries("${PROJECT_NAME}" wsock32 ws2_32)
endif()

# Optionally include brotli compression and TLS when not on windows
if (NOT WIN32)
    # Optionally enable compression
    if (ENABLE_COMPRESSION)
        include(FetchContent)
        FetchContent_Declare(
            Brotli
            GIT_REPOSITORY https://github.com/google/brotli
            GIT_TAG master
            GIT_SHALLOW 1
            GIT_PROGRESS TRUE
        )
        FetchContent_MakeAvailable(Brotli)
        target_link_libraries("${PROJECT_NAME}" ${BROTLI_LIBRARIES})
        add_definitions(-DHARBOUR_ENABLE_COMPRESSION=true)
    endif()

    if (ENABLE_TLS)
        find_package(OpenSSL REQUIRED)

        include_directories(${OPENSSL_INCLUDE_DIR})
        target_link_libraries("${PROJECT_NAME}" ${OPENSSL_LIBRARIES})
        add_definitions(-DHARBOUR_ENABLE_TLS=true)
    endif()
endif()

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
  GIT_SHALLOW 1
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(json)
target_link_libraries("${PROJECT_NAME}" nlohmann_json::nlohmann_json)