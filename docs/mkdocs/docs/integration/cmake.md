# CMake

## Integration

You can use the `harbour::harbour` interface target in CMake. This target populates the appropriate usage
requirements for [`INTERFACE_INCLUDE_DIRECTORIES`](https://cmake.org/cmake/help/latest/prop_tgt/INTERFACE_INCLUDE_DIRECTORIES.html)
to point to the appropriate include directories and [`INTERFACE_COMPILE_FEATURES`](https://cmake.org/cmake/help/latest/prop_tgt/INTERFACE_COMPILE_FEATURES.html)
for the necessary C++20 flags.

### External

To use this library from a CMake project, you can locate it directly with [`find_package()`](https://cmake.org/cmake/help/latest/command/find_package.html)
and use the namespaced imported target from the generated package configuration:

!!! example

    ```cmake title="CMakeLists.txt"
    cmake_minimum_required(VERSION 3.1)
    project(ExampleProject LANGUAGES CXX)
    
    find_package(harbour REQUIRED)
    
    add_executable(example example.cpp)
    target_link_libraries(example PRIVATE harbour::harbour)
    ```

The package configuration file, `harbourConfig.cmake`, can be used either from an install tree or directly out of
the build tree.

### Embedded

To embed the library directly into an existing CMake project, place the entire source tree in a subdirectory and call
`add_subdirectory()` in your `CMakeLists.txt` file.

!!! example

    ```cmake title="CMakeLists.txt"
    cmake_minimum_required(VERSION 3.1)
    project(ExampleProject LANGUAGES CXX)
    
    add_subdirectory(harbour)

    add_executable(example example.cpp)
    target_link_libraries(example PRIVATE harbour::harbour)
    ```

!!! note

    Do not use `#!cmake include(harbour/CMakeLists.txt)`, since that carries with it unintended consequences that
    will break the build. It is generally discouraged (although not necessarily well documented as such) to use
    `#!cmake include(...)` for pulling in other CMake projects anyways.

### FetchContent

Since CMake v3.11, [FetchContent](https://cmake.org/cmake/help/v3.11/module/FetchContent.html) can be used to
automatically download a release as a dependency at configure type.

!!! example

    ```cmake title="CMakeLists.txt"
    cmake_minimum_required(VERSION 3.11)
    project(ExampleProject LANGUAGES CXX)

    include(FetchContent)
    
    FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/griefzz/harbour
        GIT_TAG master
    )
    FetchContent_MakeAvailable(harbour)
    
    add_executable(example example.cpp)
    target_link_libraries(example PRIVATE harbour::harbour)
    ```

!!! Note

    It is recommended to use the GIT_REPOSITORY approach described above.
