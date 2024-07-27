///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file handlers.cpp
/// @brief Contains the example implementation of harbours http handlers

#include <optional>
#include <harbour/harbour.hpp>

using namespace harbour;

auto ShipsSailed() {
    static std::size_t count = 0;
    log::info("Ships sailed: {}", count++);
}

auto Yarrr() {
    return "yarrr!!!";
}

auto NoPirates(const Request &req) {
    if (auto ship = req.header("Ship-Type"))
        return (ship == "Pirate Ship") ? "No pirates allowed!" : "Ok you can pass for now...";
    else
        return "We dont know your kind!";
}

auto AddCrew(Response &resp) {
    resp["Crew"] += "Johnny The Hands, Pegleg Joe, Bob";
}

auto WalkThePlank(const Request &req, Response &resp) {
    if (req.method != http::Method::GET)
        return "I don't like your methods! Walk the plank!";
    else
        return "I'll spare yah the plank.";
}

auto main() -> int {
    Harbour hb;
    hb.dock(ShipsSailed, Yarrr, NoPirates, AddCrew, WalkThePlank);
    hb.sail();
    return 0;
}