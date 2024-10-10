///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file json.cpp
/// @brief Contains the example implementation of harbours json processing

#include <harbour/harbour.hpp>

using namespace harbour;

struct Boat {
    std::string ship;
    std::vector<std::string> crew;
    std::string captain;
};

auto Ship() -> json {
    log::info("Rendering json...");

    auto pirate = R"({ "ship": "Ol Skippy", "crew": [], "captain": "Pegleg Billy" })";
    log::warn("{}", pirate);

    auto boat = deserialize<Boat>(pirate);
    boat.crew = {"Tommy", "Johhny", "Sally"};

    return boat;
}

auto main() -> int {
    harbour::Harbour hb;
    hb.dock(Ship);
    hb.sail();
    return 0;
}