# Dock

When you want your Ship to be executed for a client you need to **Dock** your Ship.
**Dock** is Harbour's take on handler routing.

A full dock example can be found [here](https://github.com/griefzz/harbour/blob/main/examples/routes.cpp).

## Introduction

Dock comes in 2 different flavours, ```routed``` and ```global```.

A ```global``` dock will execute Ships on **any** path in the server.

A ```routed``` dock will execute Ships **only** on the specified route.

!!! example

    This is an example of **global** Ship. **Global** Ships are executed on every connection **after**
    your **routed** Ships have finished.

    ```cpp
    harbour.dock(Log);
    ```

    This is an example of a **routed** Ship. The **routed** Ship **Counter** will only be executed on the path **/counter**

    ```cpp
    harbour.dock("/counter", Counter); // dock the ship to "/counter"
    ```

!!! note

    Remember that **global** Ships will only be executed **after** your **routed** Ships. If you need a specific Ship to act before another
    consider using [Middleware](../middleware/index.md).

## Named Routes

Harbour makes it easy to caputre a specific variable in a request path. Instead of manually parsing the [Request::path](https://github.com/griefzz/harbour/blob/main/include/harbour/request/request.hpp#L65) from within a Ship,
you can use the **Named Routing** construct.

**Named Routes** are variables inside the path to your [dock](https://github.com/griefzz/harbour/blob/main/include/harbour/harbour.hpp#L98) call specified with the ':' character. 
if you specify a named route ```/some/path/:var```, once a user requests ```/some/path/foo```. The string ```foo``` and ```var``` will 
be stored inside [Request::route](https://github.com/griefzz/harbour/blob/main/include/harbour/request/request.hpp#L60) as an optional key/value pair.

A Ship held on a specified route can process this like below.

!!! example

    ```cpp
    // This Ship will print the route for a path if it exists.
    // Otherwise it will print the Request path.
    auto Routed(const Request &req) -> Response {
        // Render a named route if it exists
        if (auto route = req.route) {
            // Routes contain a key value pair describing the route
            // Key is the variable name stored on the Trie
            // Value is the parsed Request path that matches Key
            const auto &[key, value] = *route;
            return tmpl::render("{}: {}", key, value);
        }

        // Render a normal route
        return req.path;
    }

    ...
        // Named route using :name as the variable. Will accept any Method.
        hb.dock("/hello/:name", Routed);
    ...
    ```

!!! note

    You can only specify **one** named route per path. And named routes must end will their name.

    - :white_check_mark: ```/api/v1/:foo```
    - :white_check_mark: ```/api/v1/:foo/```
    - :x: ```/api/v1/:foo/:bar```
    - :x: ```/api/v1/:foo/bar```

!!! warning

    Named routes will **consume** all child routes of its path. For instance
    
    ```cpp
    // The named route will execute for all paths inside /api/v1/
    hb.dock("/api/v1/:foo", Foo);

    // Bar will never be executed since everything above 
    // /api/v1/ is consumed by the named route
    hb.dock("/api/v1/bar", Bar);
    ```

## Constraints

By default Ships will execute on **any** HTTP request method. In order to reduce the boilerplate of defining specific allowed methods
within a Ship, Harbour introduces the concept of **Method Constraints**

**Method Contraints** are used as a chainable prefix to your dock command as described below.

!!! example

    Only execute these Ships on a POST request.

    ```cpp
    hb.dock(http::Method::POST, "/foo", Foo);
    hb.dock(http::Method::POST, "/bar", Bar);
    ```

    Chain together multiple Methods using the | operator.

    ```cpp
    hb.dock(http::Method::GET | http::Method::POST, "/baz", Baz)
    ```

!!! note

    At the moment only **routed** Ships are allowed **Method Constraints**. This will change in the near future
    to support all dock methods. If you need a **global** ship to only execute on a specific method. Consider this boilerplate

    ```cpp
    if (req.method == http::Method::POST) { ... }
    ```
