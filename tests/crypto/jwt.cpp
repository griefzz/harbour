///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///

#include <string>
#include <cassert>

#include <harbour/crypto/jwt.hpp>

#define EXPECT(ok) \
    assert((ok));  \
    if (!(ok)) { return 1; }

auto main() -> int {
    using namespace harbour::crypto;
    using namespace harbour::crypto::jwt;
    using namespace harbour::json;

    const std::string secret = "12345678123456781234567812345678";

    auto token    = Token::create();
    token.payload = serialize_ordered(R"({
        "sub": "1234567890",
        "name": "John Doe",
        "iat": 1516239022
    })");


    auto j = JWT::create(secret);
    EXPECT(j);

    if (auto enc = j->encode(token)) {
        if (auto dec = j->decode(*enc)) {
            EXPECT(*dec == token);
        }
    }

    return 0;
}