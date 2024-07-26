# <center>Harbour</center>

![Alt text](docs/logo.svg "a title")

<center>A Modern C++ WebServer Framework</center>

## Table of Contents
- [Harbour](#harbour)
  - [Table of Contents](#table-of-contents)
  - [Overview](#overview)
  - [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
      - [Linux](#linux)
      - [Windows](#windows)
      - [Mac](#mac)
    - [Installation](#installation)
      - [Linux \& Mac](#linux--mac)
      - [Windows](#windows-1)
  - [Usage](#usage)
  - [Features](#features)
  - [Contributing](#contributing)

## Overview

**Harbour** is a webserver framework that gets out of your way.

## Getting Started

Execute the [deploy.sh](deploy.sh) script to pull in new changes from the repo and restart/start the webserver

### Prerequisites

You'll need a recent C++ compiler to actual compile the code. Ninja is used here on linux to reduce build times (not that they're long).

#### Linux

```bash
sudo apt install cmake git clang ninja libssl-dev
```

#### Windows

```
- C++20 compiler (clang, msvc)
- cmake
```

#### Mac

```bash
brew install cmake git clang libssl-dev
```

### Installation

#### Linux & Mac

```bash
git clone https://github.com/griefzz/harbour
chmod u+x deploy.sh
./gen_key.sh
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

All handlers have the same api:

```cpp
auto func(Server &ctx, const Request &req) -> Response;
```

The Request type holds information about a clients request and the Response type holds what we wish to send to the client. 

Part of the webserver is that any file placed inside the "ServerWebPath" will be cached and available through the Server context. Here well serve up our index.html file from the cache and deliver it to the client.

```cpp
auto IndexHandler(Server &ctx, const Request &req) -> Response {
    // check the servers cache to see if we have an /index.html file there
    if (auto index = ctx.cache["/index.html"]) {
        Response resp;
        resp.set_status(Status::Ok);
        resp.set_header("Content-Type", "text/html");
        // or like this resp["Content-Type"] = "text/html";
        resp.set_content(*index);
        return resp;
    } else {
        // if the cached file doesnt exist, send an Internal Server Error reponse
        return Status::InternalServerError;
    }
}
```

Now you can add a new Route to the webserver to handle this path

```cpp
#include <harbour/server.hpp>

auto main() -> int {
    Server server(80);
    server.route(
        Route("/", IndexHandler),
        Route("/index.html", IndexHandler));
    server.serve();

    return 0;
}
```

This is a very common task so theres a Handler that can do this for you.

```cpp
#include <harbour/server.hpp>
#include <harbour/handlers.hpp>

auto main() -> int {
    Server server(80);
    server.route(
        Route("/", Http::ServeFile("/index.html")),
        Route("/index.html", Http::ServeFile("/index.html")));
    server.serve();

    return 0;
}
```

Serving index files is *also* a very common task so there's middleware that will automatically serve up index files for all directories that arent declared routes.

```cpp
#include <harbour/server.hpp>
#include <harbour/middleware.hpp>

auto main() -> {
    Server server(80);
    server.middleware(Http::Logger,       // Log every request to console
                      Http::DefaultIndex, // Attempt to serve an index.html
                      Http::FileServer);  // Serve all files stored in our cache   
    server.serve();
}
```

Middleware all have the same api

```cpp
auto func(Server &ctx, const Request &req, Response &resp) -> void;
```

Middleware are functions that get applied to every single request.

Routes are functions that only get applied to a specific Route.

If you want to handle POST requests you can easily deserialze the post data like so:

```cpp
// Deserialze a person and send to client
auto PersonHandler(Server &ctx, const Request &req) -> Response {
    // Object to deserialize into
    struct Person {
        int age;
        std::string name;

        // format the struct to a string
        auto string() -> std::string {
            return std::format("Person ( name: {}, age: {} )\n", name, age);
        }

        // Use this macro to generate a static from_form method
        // HARBOUR_DESERIALIZABLE(Name, T1, T2, ...)
        HARBOUR_DESERIALIZABLE(Person, age, name)
    };

    // Only accept POST requests
    if (req.method == RequestMethod::POST) {
        // Attempt to deserialze a Person from a Form
        if (auto p = Person::from_form(req.form)) {
            Response resp;
            resp.set_status(Status::Ok);
            resp.set_header("Content-Type", "text/plain");
            resp.set_content(p->string());
            return resp;
        } else {
            Logger::warning("Unable to deserialize a Person!");
        }
    }

    return Status::InternalServerError;
}
```

Then simply add a route to your PersonHandler.

If you need API style routing you can do that with the following routing syntax:
```
/<name>/<age>/
```
Then name and age can be accessed from your Request in the handler like so:

```cpp
// Deserialize a Person from an API and send to the client
auto ApiHandler(Server &ctx, const Request &req) -> Response {
    // Make sure we have exactly 2 values in the route "name" and "age"
    // The route table will return exactly the specified amount
    // or 0 in the event parsing the route failed
    if (req.route.size() == 2) {
        // Get the values from the route and store them in a Person
        Person p;
        p.name = req["name"].value_or("nil");
        p.age  = std::stoi(req["age"].value_or("0"));
        // Return that person to the client
        Response resp;
        resp.set_status(Status::Ok);
        resp.set_header("Content-Type", "text/plain");
        resp.set_content(p.string());
        Logger::info(std::format("Client sent: {}", p));
        return resp;
    } else {
        Logger::warning("Unable to deserialize a Person!");
        return Status::InternalServerError;
    }
}

auto main() -> int {
    Server server(80);
    server.route(Route("/api/<name>/<age>/", ApiHandler));
    server.serve();

    return 0;
}
```

We also offer some shorthand to make routers ergnomonical to use.

```cpp
// Automatically serialize a struct into a JSON object and send to a client
auto JsonHandler(Server &ctx, const Request &req) -> Response {
    struct JsonExample {
        std::string a              = "test123";
        std::vector<int> b         = {1, 2, 3};
        std::vector<float> c       = {1.1, 2.2, 3.3};
        std::vector<std::string> d = {"test", "1", "23"};
        Person e{"Bob", 23};

        // Add the json() method for JsonExample
        HARBOUR_JSONABLE(a, b, c, d, e);
    };
    return Json(JsonExample{});
}

// Send a html hello message
auto HtmlHandler(Server &ctx, const Request &req) -> Response {
    return Html("<h1>Hello, World!</h1>");
}

// Echo back a clients request
auto EchoHandler(Server &ctx, const Request &req) -> Response {
    return Plain(req.body);
}
```

## Features

The key features for this webserver are:

- API style routing
- Middleware
- File caching
- Self hosted source
- TLS provided by OpenSSL (optional)
- Compression provided by Brotli (optional)

## Contributing

If you have interest in improving the server please send a pull request. Just ensure you maintain the same style guide, and if you must change the API, please include a reason for doing so.

