///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file loggin.cpp
/// @brief Contains the example implementation of harbours logging API

#include <harbour/harbour.hpp>

using namespace harbour;

// Ship to display a new client connection including the requested path
auto ShowClientWithPath(const Request &req) {
    log::info("{}:{} → {}", req.socket->address(), req.socket->port(), req.path);
}

// Callback for when a server warning event happens
auto OnWarning(std::shared_ptr<server::Socket> socket, const std::string_view message) {
    log::warn("{}:{} → {}", socket->address(), socket->port(), message);
}

// Callback for when a server critical event happens
auto OnCritical(std::shared_ptr<server::Socket> socket, const std::string_view message) {
    log::critical("{}:{} → {}", socket->address(), socket->port(), message);
}

auto main() -> int {
    // Assign callbacks to the server settings using the with_* methods
    auto settings = server::Settings()
                            .with_on_connection(nullptr)// Unset the connection callback
                            .with_on_warning(OnWarning)
                            .with_on_critical(OnCritical);

    // Add our callbacks to the Harbour using the settings
    Harbour hb(settings);

    // The on_connection callback happens before any processing takes effects
    // If we want to understand what the clients Request was, we need to use a Ship
    hb.dock(ShowClientWithPath);
    hb.sail();
    return 0;
}