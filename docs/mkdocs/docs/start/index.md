# Getting Started

This page will go over the basic concepts of harbour and how to create something meaningful with its api.
After going through this you should have everything you need to start building!

## Running A Server

After linking harbour as described in [Integration](../integration/index.md) you can create a main.cpp file
and begin configuring the server.

!!! example

    ```cpp title="main.cpp"
    #include <harbour/harbour.hpp>

    using namespace harbour; // for convenience

    auto main() -> int {
        // Initialize harbour
        Harbour harbour;

        // Start the server
        harbour.sail();
    }
    ```

!!! note

    By default harbour will open on port 8080. This can be changed with a server settings object for harbour.

    ```cpp
    // Create a server settings object using port 80 and construct harbour with it
    auto settings = server::Settings().with_port(80);
    Harbour harbour(settings);
    ```

## Ships

In order to do something meaningful with Harbour, we need to introduce the concept of [Ships]().
Ships are your handlers that the server will use for operating on any given [Request]().
Ships are extremely flexible functions that can return a [Response]() to the client, or nothing at all!

The most basic Ship can be done below.

!!! example

    This ship will return a text response to the client when executed.
    
    ```cpp
    auto Hello() {
        return "Hello from Harbour!";
    }
    ```

    This ship will echo whatever a client sends to the server

    ```cpp
    auto Echo(const Request &req) {
        return req.data;
    }
    ```

    This ship will optionally send a response if the client knows the password

    ```cpp
    auto Auth(const Request &req) -> std::optional<Response> {
        if (req.body == "let me in!") 
            return "Welcome!";
        return {};
    }
    ```

Ships can be any function that satisfies the [ShipConcept]().
Harbour's Ships manage their flexibility through the use of ```std::variant``` and constexpr pattern matching.

!!! example

    These are the 30 possible function signatures that satisfy the [ShipConcept]()

    ```cpp
    auto Ship(const Request &, Response &) -> void;
    auto Ship(const Request &, Response &) -> Response;
    auto Ship(const Request &, Response &) -> std::optional<Response>;
    auto Ship(const Request &, Response &) -> awaitable<Response>;
    auto Ship(const Request &, Response &) -> awaitable<std::optional<Response>>;
    auto Ship(const Request &, Response &) -> awaitable<void>;

    auto Ship(Response &, const Request &) -> void;
    auto Ship(Response &, const Request &) -> Response;
    auto Ship(Response &, const Request &) -> std::optional<Response>;
    auto Ship(Response &, const Request &) -> awaitable<Response>;
    auto Ship(Response &, const Request &) -> awaitable<std::optional<Response>>;
    auto Ship(Response &, const Request &) -> awaitable<void>;

    auto Ship(const Request &) -> void;
    auto Ship(const Request &) -> Response;
    auto Ship(const Request &) -> std::optional<Response>;
    auto Ship(const Request &) -> awaitable<Response>;
    auto Ship(const Request &) -> awaitable<std::optional<Response>>;
    auto Ship(const Request &) -> awaitable<void>;

    auto Ship(Response &) -> void;
    auto Ship(Response &) -> Response;
    auto Ship(Response &) -> std::optional<Response>;
    auto Ship(Response &) -> awaitable<Response>;
    auto Ship(Response &) -> awaitable<std::optional<Response>>;
    auto Ship(Response &) -> awaitable<void>;

    auto Ship() -> void;
    auto Ship() -> Response;
    auto Ship() -> std::optional<Response>;
    auto Ship() -> awaitable<Response>;
    auto Ship() -> awaitable<std::optional<Response>>;
    auto Ship() -> awaitable<void>;

    ```

Ships are an extremely powerful and core concept in harbour. They maintain excellent flexibility while being as simple
as possible to create. Harbour does its best to make the API as simple as possible so you can spend less time reading docs
and more time building your project.

## Docking

In order to use your Ships with harbour you must [Dock]() them.
Dock comes in 2 different flavours, ```routed``` and ```global```.

A ```global``` dock will execute Ships on **any** path in the server.

A ```routed``` dock will execute Ships **only** on the specified route.

!!! example

    This is an example of global Ship.

    ```cpp
    // Print the address and port of a new connection
    auto Log(const Request &req) {
        // logln will pretty print our prompt to std::clog with a new line
        log::info("Connection: {}:{}", req.socket.address(), req.socket.port());
    }

    auto main() -> int {
        Harbour harbour;
        harbour.dock(Log); // Globally dock the ship
        harbour.sail();
    }
    ```

    This is an example of a routed Ship

    ```cpp
    // Return the number of visits to this Ship
    auto Counter() {
        // Use an atomic variable since multiple connections 
        // could request this value at once
        static std::atomic<std::size_t> n = 0;
        n++;

        // Convert the std::size_t to a std::string and return it
        return std::to_string(n.load());
    }

    auto main() -> int {
        Harbour harbour;
        harbour.dock("/counter", Counter); // dock the ship to "/counter"
        harbour.sail();
    }
    ```

!!! note

    ```dock()``` also accepts any number of ships as parameters and returns a reference to Harbour. It's encouraged to use dock this way.

    ```cpp
    harbour.dock(foo_a, foo_b)
           .dock(bar_a, bar_b)
           .dock(baz_a, baz_b, baz_c);
    ```

    Chaining is not required and multiple dock calls can be done at any time!

!!! warning

    It's important to note that Ships are executed in a FIFO order and will exit at the first instance of a Ship
    returning a valid Response.

    ```cpp
    auto Foo() -> void { ... };

    // An empty optional is *not* a valid Response
    auto Bar() -> std::optional<Response> { return {}; }

    // This *is* considered a valid Response
    auto Baz() -> Response { return "Baz"; } 

    auto Boz() -> void { ... }

    auto main() -> int {
        ...
        // Baz will execute since an empty optional is not a *valid* Response.
        // Boz will never be reached since Baz *does* return a Response 
        hb.dock(Foo, Bar, Baz, Boz);
        ...
    }
    ```

    If you want to modify a Response between Ships you should use the Response& parameter in your Ships.

    If no Ship returns a valid Response. The Response& parameter will ultimately be returned instead.


## Start Building

That should cover most of the important concepts in harbour and have you prepared to start building!

## Features

Harbour is batteries included!

- First class [JSON]() support through the bespoke [nlohmann::json](https://json.nlohmann.me/) library.
- [Secure cookies]() using [AES256]() and [HMAC]() authentication.  
- Template rendering provided by [fmtlib](). 

Head over to [Features](../features/index.md) to get an overview of what Harbour has to offer!

## API

Generate the full documentation for Harbour's API using doxygen.
