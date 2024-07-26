///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file files.cpp
/// @brief Contains the example implementation of using files with harbour

#include <harbour/harbour.hpp>

using namespace harbour;

auto Ship(const Request &req) -> Response {
    log::info("Loading file...");

    if (auto file = tmpl::load_file(req.path))
        return *file;

    log::warn("Couldnt find file: {}", req.path);
    return http::Status::NotFound;
}

int main() {
    Harbour hb;
    hb.dock(Ship);
    hb.sail();
    return 0;
}