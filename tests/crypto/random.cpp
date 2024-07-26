///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///

#include <iostream>

#include <harbour/crypto/random.hpp>

auto main() -> int {
    if (auto key = harbour::crypto::random::bytes(32))
        return 0;

    if (auto key = harbour::crypto::random::bytes_secure(32))
        return 0;

    if (auto key = harbour::crypto::random::string(32))
        return 0;

    if (auto key = harbour::crypto::random::string_secure(32))
        return 0;

    return 1;
}