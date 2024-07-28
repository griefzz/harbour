///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file routes.cpp
/// @brief Contains the example implementation of harbours route handling

#include <harbour/harbour.hpp>

using namespace harbour;

// This Ship will log the ip:port of our client and the Request path
auto Global(const Request &req) {
    log::info("{}:{} → {}", req.socket->address(), req.socket->port(), req.path);
}

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

int main() {
    Harbour hb;

    // Since no path is provided this Ship is docked globally
    // and will execute on each Request after routed ships have finished
    hb.dock(Global);

    // Named route using :name as the variable. Will accept any Method.
    hb.dock("/hello/:name", Routed);

    // Named route with a Method constraint using :id as the variable.
    // A Method constraint will only execute Ships on the route if the
    // Request method matches the constraint.
    // In this case /get/:id will only be served on a http::Method::GET.
    hb.dock("GET /get/:id", Routed);

    // Named route with a Method contraint to only accept an http::Method::POST.
    hb.dock("POST /post", Routed);

    // Routes are automatically converted to start and end with a '/'
    hb.dock("123/456", Routed);

    hb.sail();
    return 0;
}