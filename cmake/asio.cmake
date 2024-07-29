include(FetchContent)

FetchContent_Declare(asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG master
)
FetchContent_GetProperties(asio)

if(NOT asio_POPULATED AND NOT TARGET asio)
    FetchContent_MakeAvailable(asio)

    # Find Version ##
    file(STRINGS "${asio_SOURCE_DIR}/asio/include/asio/version.hpp" ASIO_VERSION
        REGEX "^#define ASIO_VERSION [0-9]+"
    )
    string(REGEX REPLACE "^#define ASIO_VERSION [0-9]+ // ([0-9.]+)$" "\\1" ASIO_VERSION
        "${ASIO_VERSION}"
    )

    set(ASIO_VERSION
        ${ASIO_VERSION}
        CACHE STRING "asio library version" FORCE
    )

    # Configure ##
    add_library(asio STATIC ${asio_SOURCE_DIR}/asio/src/asio.cpp)
    add_library(asio::asio ALIAS asio)
    target_include_directories(
        asio PUBLIC $<BUILD_INTERFACE:${asio_SOURCE_DIR}/asio/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    find_package(Threads REQUIRED QUIET)
    target_link_libraries(asio PUBLIC Threads::Threads)

    target_compile_features(asio PUBLIC cxx_std_20)
    target_compile_definitions(
        asio PUBLIC -DASIO_STANDALONE -DASIO_SEPARATE_COMPILATION -DASIO_NO_DEPRECATED
    )

    # Fix warning : "Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately."
    # https://stackoverflow.com/questions/9742003/platform-detection-in-cmake
    if(WIN32 AND CMAKE_SYSTEM_VERSION)
        set(ver ${CMAKE_SYSTEM_VERSION})
        string(REPLACE "." "" ver ${ver})
        string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})
        set(version "0x${ver}")
        target_compile_definitions(asio PUBLIC -D_WIN32_WINNT=${version})
    endif()
endif()

target_link_libraries(harbour INTERFACE asio::asio)
