# Logging

Harbour has a very minimal yet useful [logging API](https://github.com/griefzz/harbour/blob/main/include/harbour/log.hpp). Since logging itself can bring in a myriad
of design considerations and requirements per project, Harbour instead opts to simply provide a
set of pretty-printed log functions wrapping fmtlib. The user is encouraged to include their own logging library
for doing complex logging.

## Functions

Harbour provides 3 different [log](https://github.com/griefzz/harbour/blob/main/include/harbour/log.hpp#L20) functions.

- [log::info](https://github.com/griefzz/harbour/blob/main/include/harbour/log.hpp#L36) - This function is for normal information.
- [log::warn](https://github.com/griefzz/harbour/blob/main/include/harbour/log.hpp#L56) - This function is for unexpected but recoverable issues.
- [log::critical](https://github.com/griefzz/harbour/blob/main/include/harbour/log.hpp#L76) - This function is for catastrohpic issues that may or may not be recoverable.

## Usage

!!! example

    The log functions are wrappers using fmtlib so any type that is implemented by a fmt::formatter works here.
    
    Some examples of possible usage.

    ```cpp
    log::info("Connection: {}:{}", ip, port);
    log::warn("Something unexpected happened: {}", e.what());
    log::critical("Everything is on fire! Send help!");
    ```
