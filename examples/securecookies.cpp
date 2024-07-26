///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file securecookies.cpp
/// @brief Contains the example implementation of a secure cookies encoder with harbour

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
    <title>Harbour | Secure Cookies Example</title>
</head>
<body>
    <h1>Cookies:</h1>
    {}
</body>
</html>)";

struct Session {
    SecureCookies &securecookies;///< Secure cookie encoder

    auto operator()(const Request &req) -> Response {
        // Create a Cookies object from a request.
        // When creating a Cookies object from another source
        // we need to handle the possibility of a parsing error
        if (auto cookies = Cookies::create(req)) {
            // Grab the current session if it exists
            if (auto data = cookies->get("example-session")) {
                // Decode the session data into our existing map
                if (securecookies.decode("example-session", *data, cookies->data)) {
                    // Show the user the encoded and decoded cookies
                    return tmpl::render(index_tmpl, cookies->string());
                }
            }
        }

        // Create an empty Cookie object to store our cookie data
        // Any object that can be used with msgpack::{pack,unpack} works
        cookies::Map data;
        data["id"]   = "123";
        data["name"] = "bob";

        // Encode the cookies
        auto encoded = securecookies.encode("example-session", data);
        if (encoded.empty()) {
            log::warn("Failed to encode cookies!");
            return http::Status::InternalServerError;
        }

        // Create a new Cookies object and store the encoded data
        auto cookies = Cookies::create()
                               .with_http_only()                                       // Only allow HTTP Responses to set cookies
                               .with_secure()                                          // Only allow this cookie on SSL connections
                               .with_same_site_policy(cookies::SameSitePolicy::Strict);// Only allow this cookie on the origin domain

        // Store the encoded session data
        cookies["example-session"] = encoded;

        // Respond to the client including our cookies
        log::info("Setting cookies: {}", cookies.string());
        return Response(http::Status::OK)
                .with_cookies(cookies)
                .with_data(tmpl::render(index_tmpl, "Reload to show cookies!"));
    }
};

auto main() -> int {
    // Create a secure cookies encoder
    auto securecookies = SecureCookies::create();
    if (!securecookies) {
        log::critical("Failed to create secure cookie storage!");
        return 1;
    }

    auto settings = server::Settings().with_ssl_paths("cert.pem", "pkey.pem");

    Harbour hb(settings);
    // Create/show secure cookies to the user
    hb.dock("/", Session{*securecookies});// Pass in our secure cookie encoder as Cargo
    hb.sail();
    return 0;
}