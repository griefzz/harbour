///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file templates.cpp
/// @brief Contains the example implementation of harbours html templating

#include <harbour/harbour.hpp>

using namespace harbour;

auto Hello() {
    log::info(tmpl::render("Hello from {}!", "Harbour"));
}

auto Ship() {
    log::info("Rendering template...");
    return tmpl::render_file("index.tmpl", "Harbour", "<h1>Ahoy!</h1>")
            .value_or("Couldn't compile template.");
}

auto main() -> int {
    Harbour hb;
    hb.dock(Hello, Ship);
    hb.sail();
    return 0;
}