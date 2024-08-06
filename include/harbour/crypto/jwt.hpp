///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file jwt.hpp
/// @brief Contains the implementation details for Harbours JSON Web Token API

#pragma once

#include <optional>
#include <string>
#include <array>

#include "base64url.hpp"
#include "hmac.hpp"
#include "random.hpp"
#include "../ranges.hpp"
#include "../json.hpp"

namespace harbour {
    namespace crypto {
        namespace jwt {

            using ranges::RandomAccessScalarRange;

            /// @brief JSON Web Token (https://jwt.io/introduction)
            struct Token {
                json::ordered_json_t header; ///< Header for JWT
                json::ordered_json_t payload;///< Payload for JWT

                friend inline auto operator==(const Token &lhs, const Token &rhs) -> bool {
                    return lhs.header == rhs.header &&
                           lhs.payload == rhs.payload;
                }

                friend inline auto operator!=(const Token &lhs, const Token &rhs) -> bool {
                    return !(lhs == rhs);
                }

                HARBOUR_JSONABLE(Token, header, payload);
            };

            /// @brief Json Web Token Encoder/Decoder
            struct JWT {
                std::array<char, 32> secret;///< 256-bit secret key for encode/decode

                /// @brief
                /// @param key
                /// @return
                [[nodiscard]] static auto create(RandomAccessScalarRange auto &&key) -> std::optional<JWT> {
                    if (key.size() != 32) return {};
                    JWT j;
                    std::ranges::copy(key.begin(), key.end(), j.secret.begin());
                    return j;
                }

                /// @brief
                /// @return
                [[nodiscard]] static auto create() -> std::optional<JWT> {
                    if (auto key = crypto::random::bytes(32))
                        return JWT::create(*key);

                    return {};
                }

                /// @brief
                /// @param token
                /// @return
                [[nodiscard]] auto encode(const jwt::Token &token) -> std::optional<std::string> {
                    if (auto h = crypto::base64url::encode(token.header.dump())) {
                        if (auto p = crypto::base64url::encode(token.payload.dump())) {
                            auto enc = *h + "." + *p;
                            if (auto s = crypto::hmac::sign(enc, secret)) {
                                return enc + "." + *s;
                            }
                        }
                    }

                    return {};
                }

                /// @brief
                /// @param src
                /// @return
                [[nodiscard]] auto decode(RandomAccessScalarRange auto &&src) -> std::optional<jwt::Token> {
                    const auto sep = std::string_view(".");

                    auto parts = src | std::views::split(sep) | ranges::to<std::vector>;
                    if (parts.size() != 3)
                        return {};

                    auto header  = std::string(parts[0].begin(), parts[0].end());
                    auto payload = std::string(parts[1].begin(), parts[1].end());
                    auto mac     = std::string_view(parts[2].begin(), parts[2].end());

                    if (auto ok = crypto::hmac::verify(src, mac, secret); !ok)
                        return {};

                    Token token;

                    auto dec_header = crypto::base64url::decode(header);
                    if (!dec_header)
                        return {};

                    auto dec_payload = crypto::base64url::decode(payload);
                    if (!dec_payload)
                        return {};

                    token.header  = json::ordered_serialize(*dec_header);
                    token.payload = json::ordered_serialize(*dec_payload);

                    return token;
                }
            };

        }// namespace jwt
    }// namespace crypto
}// namespace harbour