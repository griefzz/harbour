///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///

#include <string>
#include <cassert>

#include <harbour/crypto/base64.hpp>
#include <harbour/crypto/random.hpp>

auto main() -> int {
    const std::string valid   = harbour::crypto::random::string(103).value_or("");
    const std::string invalid = "";

    // test invalid
    if (auto enc = harbour::crypto::base64::encode(invalid))
        return 1;

    if (auto dec = harbour::crypto::base64::decode(invalid))
        return 1;

    // test valid
    if (auto enc = harbour::crypto::base64::encode(valid))
        if (auto dec = harbour::crypto::base64::decode(*enc))
            return 0;

    return 1;
}