include(FetchContent)

FetchContent_Declare(reflectcpp
    GIT_REPOSITORY https://github.com/getml/reflect-cpp
    GIT_TAG main
)
FetchContent_MakeAvailable(reflectcpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(reflectcpp PRIVATE -w) # Suppress all warnings

# target_compile_options(your_target PRIVATE -Wno-specific-warning) # Suppress specific warning
elseif(MSVC)
    target_compile_options(reflectcpp PRIVATE /w) # Suppress all warnings

    # target_compile_options(your_target PRIVATE /wd<warning-number>) # Suppress specific warning
endif()

target_link_libraries(harbour INTERFACE reflectcpp)