///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///

#include <string>
#include <cassert>

#include <harbour/crypto/base64url.hpp>
#include <harbour/crypto/random.hpp>

#define EXPECT(ok) \
    assert((ok));  \
    if (!(ok)) { return 1; }

auto main() -> int {
    const std::string payload = R"({"alg":"HS256","typ":"JWT"})";

    auto valid = harbour::crypto::random::string(103);
    EXPECT(valid);
    const std::string invalid = "";

    // test invalid
    auto enc = harbour::crypto::base64url::encode(invalid);
    EXPECT(!enc);

    auto dec = harbour::crypto::base64url::decode(invalid);
    EXPECT(!dec);

    // test valid
    enc = harbour::crypto::base64url::encode(*valid);
    EXPECT(enc);
    dec = harbour::crypto::base64url::decode(*enc);
    EXPECT(dec);
    EXPECT(*dec == *valid);

    enc = harbour::crypto::base64url::encode(payload);
    EXPECT(enc);
    dec = harbour::crypto::base64url::decode(*enc);
    EXPECT(dec);
    EXPECT(*dec == payload);

    return 0;
}