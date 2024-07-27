///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file routes.cpp
/// @brief Contains the example implementation of harbours route handling

#include <harbour/harbour.hpp>

using namespace harbour;

auto Route(const Request &req) -> Response {
    if (auto route = req.route) {
        const auto &[key, path] = *route;
        return tmpl::render("{}: {}", key, path);
    }

    return http::Status::InternalServerError;
}

auto main() -> int {
    Harbour hb;
    hb.dock("/a/b/c/d", Route);
    hb.dock("/aples/b/c/d", Route);
    hb.dock("/api/b/c/d", Route);
    hb.dock("/b/c/d", Route);

    hb.sail();
    return 0;
}