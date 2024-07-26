include(FetchContent)

FetchContent_Declare(unordered_dense
    GIT_REPOSITORY https://github.com/martinus/unordered_dense
    GIT_TAG main
)
FetchContent_MakeAvailable(unordered_dense)

target_link_libraries(harbour INTERFACE unordered_dense::unordered_dense)