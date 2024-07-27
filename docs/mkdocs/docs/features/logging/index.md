# Logging

Harbour has a very minimal yet useful [logging API](https://github.com/griefzz/harbour/blob/main/include/harbour/log.hpp). Since logging itself can bring in a myriad
of design considerations and requirements per project, Harbour instead opts to simply provide a
set of pretty-printed log functions wrapping fmtlib and callbacks to customize for your needs.

Harbour always aims to provide you with as much flexibility as possible without making things complicated and logging
is no different.

## Functions

Harbour provides 3 different [log](https://github.com/griefzz/harbour/blob/main/include/harbour/log.hpp#L20) functions.

- [log::info](https://github.com/griefzz/harbour/blob/main/include/harbour/log.hpp#L36) - This function is for normal information.
- [log::warn](https://github.com/griefzz/harbour/blob/main/include/harbour/log.hpp#L56) - This function is for unexpected but recoverable issues.
- [log::critical](https://github.com/griefzz/harbour/blob/main/include/harbour/log.hpp#L76) - This function is for catastrohpic issues that may or may not be recoverable.

!!! example

    The log functions are wrappers using fmtlib so any type that is implemented by a fmt::formatter works here.
    
    Some examples of possible usage.

    ```cpp
    log::info("Connection: {}:{}", ip, port);
    log::warn("Something unexpected happened: {}", e.what());
    log::critical("Everything is on fire! Send help!");
    ```

## Callbacks

To enable your own logging solutions harbour provides 3 callback coroutines for important server events.

- ```on_connection``` - Callback for when a new connection happens.
- ```on_warning``` - Callback for when a server warning event happens.
- ```on_critical``` - Callback for when a server critical event happens.

!!! example

    This example sets callbacks in a server::Settings object then passes them into a Harbour instance.

    ```cpp
    // For convenience
    using harbour::server::SharedSocket;

    // Callback for when a server warning event happens
    auto OnConnection(SharedSocket socket, const std::string_view message) -> awaitable<void> {
        log::info("{}:{} → {}", socket->address(), socket->port(), message);
        co_return;
    }

    // Callback for when a server warning event happens
    auto OnWarning(SharedSocket socket, const std::string_view message) -> awaitable<void> {
        log::warn("{}:{} → {}", socket->address(), socket->port(), message);
        co_return;
    }

    // Callback for when a server critical event happens
    auto OnCritical(SharedSocket socket, const std::string_view message) -> awaitable<void> {
        log::critical("{}:{} → {}", socket->address(), socket->port(), message);
        co_return;
    }

    auto main() -> int {
        // Assign callbacks to the server settings using the with_* methods
        auto settings = server::Settings()
                                .with_on_connection(OnConnection)
                                .with_on_warning(OnWarning)
                                .with_on_critical(OnCritical);

        // Add our callbacks to the Harbour using the settings
        Harbour hb(settings);
    }
    ```

A common instance of working with callbacks would be to use a global Ship to log all paths requested on a server.

!!! example

    This example will unset the default on_connection callback and instead use a global Ship in its place

    ```cpp
    auto ShowClientWithPath(const Request &req) {
        log::info("{}:{} → {}", req.socket->address(), req.socket->port(), req.path);
    }
    
    auto main() -> int {
        // Disable the default on_connection callback and use a Ship instead
        auto settings = server::Settings().with_on_connection(nullptr);
        Harbour hb(settings);
        hb.dock(ShowClientWithPath);
        hb.sail();
    ```

    Now whenever a client's Request is processed, we'll log the ip and port of the client and the requested path.

The on_connection callback is performed before a Request is parsed. So this is a common pattern when you want more fine-grained logging
of a clients connection.

!!! warning

    While these callbacks are coroutines, their execution is awaited inside the server. 
    It's important to note that if you do things such as
    file or network IO inside a callback that harbour will halt a clients processing until they finish. 
    It's preferable for such tasks to be deffered asynchronously for best performance.

!!! note

    By default Harbour sets these callbacks to good defaults. Using your own callbacks
    should be reserved for specialty logging libraries or building metrics.
