include(FetchContent)
set(FMT_OS OFF)
set(FMT_INSTALL OFF)
FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG master
)
FetchContent_MakeAvailable(fmt)
target_link_libraries(harbour INTERFACE fmt::fmt)