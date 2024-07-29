///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///

#include <iostream>
#include <format>
#include <unordered_map>
#include <cassert>

#include <harbour/harbour.hpp>

#define EXPECT(ok) \
    assert((ok));  \
    if (!(ok)) return 1;

auto main() -> int {
    if (auto securecookie = harbour::SecureCookies::create()) {
        std::unordered_map<std::string, std::string> map = {{"1", "1"}, {"2", "2"}, {"3", "3"}};
        decltype(map) rmap;
        auto enc = securecookie->encode("session-name", map);
        auto ok  = securecookie->decode("session-name", enc, rmap);
        if (ok) {
            EXPECT(map == rmap);
            return 0;
        }
    }

    return 1;
}