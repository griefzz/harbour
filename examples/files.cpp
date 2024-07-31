///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file files.cpp
/// @brief Contains the example implementation of using files with harbour

#include <harbour/harbour.hpp>

using namespace harbour;

// Load a file and return it to the client
auto Load(const Request &req) -> Response {
    log::info("Loading file...");

    if (auto file = tmpl::load_file(req.path))
        return *file;

    log::warn("Couldnt find file: {}", req.path);
    return http::Status::NotFound;
}

// Load a file asynchronously and return it to the client
auto AsyncLoad(const Request &req) -> awaitable<Response> {
    log::info("Async loading file...");

    // Await the result of an async load_file
    // First parameter is the file to load
    // Second parameter is the completion token
    if (auto file = co_await tmpl::load_file_async(req.path, use_awaitable))
        co_return *file;

    log::warn("Couldnt find file: {}", req.path);
    co_return http::Status::NotFound;
}

auto main() -> int {
    Harbour hb;
    hb.dock(Load);
    // hb.dock(AsyncLoad);
    hb.sail();
    return 0;
}