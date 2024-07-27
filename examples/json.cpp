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

    HARBOUR_JSONABLE(Boat, ship, crew, captain);
};

auto Ship() -> json::json_t {
    log::info("Rendering json...");

    auto pirate    = json::serialize(R"({ "ship": "Ol Skippy", "captain": "Pegleg Billy" })");
    pirate["crew"] = {"Bobby", "Johhny", "Tommy"};
    log::info(pirate.dump());

    Boat boat;
    json::deserialize(pirate, boat);

    return boat;
}

auto main() -> int {
    Harbour hb;
    hb.dock(Ship);
    hb.sail();
    return 0;
}