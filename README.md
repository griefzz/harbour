# Griefz Space

A C++ webserver and personal website.

## Table of Contents
- [Overview](#overview)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
- [Usage](#usage)
- [Features](#features)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgments](#acknowledgments)

## Overview

The webserver itself is pretty straightforward. All configuration is done inside [config.h](/src/config.h). The server itself does not support TLS or any method other than GET for somplicities sake.

## Getting Started

Execute the [deploy.sh](deploy.sh) script to pull in new changes from the repo and restart/start the webserver

### Prerequisites

You'll need a recent C++ compiler to actual compile the code. Ninja is used here on linux to reduce build times (not that they're long).

#### Linux

```bash
sudo apt install cmake git build-essential ninja
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
git clone https://github.com/griefzz/griefzspace
chmod u+x deploy.sh
./deploy.sh
```

#### Windows

```bash
git clone https://github.com/griefzz/griefzspace
mkdir build && cd build
cmake ..
# Build the .vcproj file
```

## Usage

Using the server as a library is also very straight forward.

```cpp
#include "server.h"

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
        resp.set_content(*index);
    } else {
        // if the cached file doesnt exist, send an Internal Server Error reponse
        resp = Response(ResponseType::InternalServerError);
    }
}
```

Now you can add a new Route to the webserver to handle this path

```cpp
#include "server.h"

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
#include "server.h"
#include "handlers.h"

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
#include "server.h"
#include "middleware.h"

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

## Features

The key features for this webserver are:

- Routing
- Middleware
- File caching
- Self hosted source
- No third-party libraries

## Contributing

If you have interest in improving the server please send a pull request. Just ensure you maintain the same style guide, and if you must change the API, please include a reason for doing so.

