# Harbour

A C++ webserver and personal website hosting its own source code.

## Table of Contents
- [Overview](#overview)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
- [Usage](#usage)
- [Features](#features)
- [Contributing](#contributing)

## Overview

The webserver itself is pretty straightforward. All configuration is done inside [config.h](/src/config.h). The server itself does not support TLS, but it does handle GET and POST.

## Getting Started

Execute the [deploy.sh](deploy.sh) script to pull in new changes from the repo and restart/start the webserver

### Prerequisites

You'll need a recent C++ compiler to actual compile the code. Ninja is used here on linux to reduce build times (not that they're long).

#### Linux

```bash
sudo apt install cmake git clang ninja
```

#### Windows

```
- C++20 compiler (clang, msvc)
- cmake
```

#### Mac

```bash
brew install cmake git clang
```

### Installation

#### Linux & Mac

```bash
git clone https://github.com/griefzz/harbour
chmod u+x deploy.sh
./deploy.sh
```

#### Windows

```bash
git clone https://github.com/griefzz/harbour
mkdir build && cd build
cmake ..
# Build the .vcproj file
```

## Usage

Using the server as a library is also very straight forward.

```cpp
#include "server.hpp"

auto main() -> int {
    Server server(80);
    server.serve();

    return 0;
}
```

This will launch the server and listen on port 80 for connections. This isnt very useful though since we dont have any handlers or middleware to do anything with the connections.

All middleware and handlers have the same api:

```cpp
auto func(Server &ctx, const Request &req, Response &resp) -> void;
```

The Request type holds information about a clients request and the Response type holds what we wish to send to the client. 

Part of the webserver is that any file placed inside the "ServerWebPath" will be cached and available through the Server context. Here well serve up our index.html file from the cache and deliver it to the client.

```cpp
auto IndexHandler(Server &ctx, const Request &req, Response &resp) -> void {
    // check the servers cache to see if we have an /index.html file there
    if (auto index = ctx.cache["/index.html"]) {
        resp.set_type(ResponseType::Ok);
        resp.set_header("Content-Type", "text/html");
        // or like this resp["Content-Type"] = "text/html";
        resp.set_content(*index);
    } else {
        // if the cached file doesnt exist, send an Internal Server Error reponse
        resp = Response(ResponseType::InternalServerError);
    }
}
```

Now you can add a new Route to the webserver to handle this path

```cpp
#include "server.hpp"

auto main() -> int {
    Server server(80);
    server.route(
        Route{"/", IndexHandler},
        Route{"/index.html", IndexHandler});
    server.serve();

    return 0;
}
```

This is a very common task so theres a Handler that can do this for you.

```cpp
#include "server.hpp"
#include "handlers.hpp"

auto main() -> int {
    Server server(80);
    server.route(
        Route{"/", Handlers::ServeFile("/index.html")},
        Route{"/index.html", Handlers::ServeFile("/index.html")});
    server.serve();

    return 0;
}
```

Serving index files is *also* a very common task so there's middleware that will automatically serve up index files for all directories that arent declared routes.

```cpp
#include "server.hpp"
#include "middleware.hpp"

auto main() -> {
    Server server(80);
    server.middleware(Middleware::Logger,       // Log every request to console
                      Middleware::DefaultIndex, // Attempt to serve an index.html
                      MiddleWare::FileServer);  // Serve all files stored in our cache   
    server.serve();
}
```

Middleware are functions that get applied to every single request.

Routes are functions that only get applied to a specific Route.

If you want to handle POST requests you can easily deserialze the post data like so:

```cpp
// Deserialze a person and send to client
auto PersonHandler(Server &ctx, const Request &req, Response &resp) -> void {
    // Object to deserialize into
    struct Person {
        int age;
        std::string name;

        // format the struct to a string
        auto string() -> std::string {
            return std::format("Person ( name: {}, age: {} )\n", name, age);
        }

        // Use this macro to generate a static from_form method
        // SERVER_DESERIALIZABLE(Name, T1, T2, ...)
        SERVER_DESERIALIZABLE(Person, age, name)
    };

    // Only accept POST requests
    if (req.method == RequestMethod::POST) {
        // Attempt to deserialze a Person from a Form
        if (auto p = Person::from_form(req.form)) {
            resp.set_type(ResponseType::Ok);
            resp.set_header("Content-Type", "text/plain");
            resp.set_content(p->string());
        } else {
            Logger::warning("Unable to deserialize a Person!");
        }
    }
}
```

Then simply add a route to your PersonHandler.

## Features

The key features for this webserver are:

- Routing
- Middleware
- File caching
- Self hosted source
- No third-party libraries

## Contributing

If you have interest in improving the server please send a pull request. Just ensure you maintain the same style guide, and if you must change the API, please include a reason for doing so.

