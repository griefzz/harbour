///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///

#include <string>
#include <cassert>

#include <harbour/crypto/aes.hpp>

#define EXPECT(ok) \
    assert((ok));  \
    if (!(ok)) return 1;

auto main() -> int {
    std::string plain = "password";
    std::string key   = "12341234123412341234123412341234";

    if (auto enc = harbour::crypto::aes256::encrypt(plain, key)) {
        if (auto dec = harbour::crypto::aes256::decrypt(*enc, key)) {
            EXPECT(*dec == plain);
            return 0;
        }
    }

    return 1;
}