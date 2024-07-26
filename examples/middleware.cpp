///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file middleware.cpp
/// @brief Contains the example implementation of harbours middleware handling

#include <harbour/harbour.hpp>

using namespace harbour;

// Show an admin panel to the user
auto AdminPanel() {
    return "<h1>Welcome Admin!</h1>";
}

// Create a Middleware to perform authentication for Ships
auto Auth(const Request &req) -> std::optional<Response> {
    // Check the request header for a password
    // and attempt to authenticate the user.
    // Returning nothing here means the user was 
    // authenticated and the rest of our Ships should processs
    if (auto password = req.header("Password"))
        if (*password == "super secret password")
            return {};

    // If authentication failed return a Forbidden status code
    // and report the incident to the event log
    log::warn("Client was not authenticated!");
    return http::Status::Forbidden;
}

auto main() -> int {
    Harbour hb;
    // Add a Middleware to the AdminPanel.
    // Middleware get executed before every specified Ship.
    // If your Middleware returns a valid response, it will
    // be sent to the user without processing the rest of the Ships in the chain
    hb.dock("/admin", Middleware(Auth, AdminPanel));
    hb.sail();

    return 0;
}