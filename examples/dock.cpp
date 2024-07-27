///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file routes.cpp
/// @brief Contains the example implementation of harbours route handling

#include <harbour/harbour.hpp>

using namespace harbour;

auto Hello() {
    return "Hello!";
}

struct AddDock {
    Harbour &hb;

    auto operator()() {
        hb.dock("/test", Hello);
        return "Added Dock";
    }
};

auto main() -> int {
    Harbour hb;
    hb.dock("/", AddDock{hb});
    hb.sail();
    return 0;
}