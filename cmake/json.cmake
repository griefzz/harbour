include(FetchContent)

FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG master
)
FetchContent_MakeAvailable(json)

target_link_libraries(harbour INTERFACE nlohmann_json::nlohmann_json)