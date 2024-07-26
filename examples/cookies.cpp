///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file cookies.cpp
/// @brief Contains the example implementation of cookies with harbour

#include <iostream>
#include <unordered_map>

#include <harbour/harbour.hpp>

using namespace harbour;

// index.html
const std::string index_tmpl = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Harbour | Cookies Example</title>
</head>
<body>
    <h1>Cookies:</h1>
    {}
</body>
</html>)";

// If the client has cookies already, show them.
// Otherwise we set the cookies for the client
auto GetSetCookies(const Request &req) -> Response {
    // Create a Cookies object from a request.
    // When creating a Cookies object from another source
    // we need to handle the possibility of a parsing error
    if (auto cookies = Cookies::create(req))
        return tmpl::render(index_tmpl, cookies->string());

    // Create an empty Cookies object
    auto cookies = Cookies::create();

    // The Cookies object acts as a map
    cookies["id"]   = "123";
    cookies["name"] = "bob";

    // Respond to the client including our cookies
    log::info("Setting cookies: {}", cookies.string());
    return Response(http::Status::OK)
            .with_cookies(cookies)
            .with_data(tmpl::render(index_tmpl, "Reload to show cookies!"));
}

int main() {
    Harbour hb;
    hb.dock("/", GetSetCookies);// Create/show cookies to the user
    hb.sail();
    return 0;
}