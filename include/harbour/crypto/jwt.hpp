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

namespace harbour::crypto::jwt {

    using ranges::RandomAccessScalarRange;

    /// @brief JSON Web Token (https://jwt.io/introduction)
    struct Token {
        struct Header {
            std::string alg;
            std::string typ;
            
            auto operator<=>(const Header &) const = default;
        };

        Header header;
        rfl::Generic::Object payload;

        /// @brief Create a new Json Web Token with the default header
        ///        Harbour's JWT Encoder/Decoder assumes the algo
        ///        to be HS256 and this provides a convenient way to
        ///        check if a recieved Token matches our defaults
        /// @return Token with default header parameters
        static auto create() -> Token {
            Token t;
            t.header = deserialize<Token::Header>(R"({"alg": "HS256","typ": "JWT"})");
            return t;
        }

        bool operator==(const Token &other) const {
            return header == other.header && serialize(payload) == serialize(other.payload);
        }
    };

    /// @brief Json Web Token Encoder/Decoder
    struct JWT {
        std::array<char, 32> secret;///< 256-bit secret key for encode/decode

        /// @brief Create a new JSON Web Token Encoder/Decoder Object
        /// @param key 256-bit secret key for encoding/decoding
        /// @return std::optional<JWT> JWT on success, empty on error
        [[nodiscard]] static auto create(RandomAccessScalarRange auto &&key) -> std::optional<JWT> {
            if (key.size() != 32) return {};
            JWT j;
            std::ranges::copy(key.begin(), key.end(), j.secret.begin());
            return j;
        }

        /// @brief Create a new JSON Web Token Encoder/Decoder Object
        ///        using a randomly generated 256-bit secret key
        /// @return std::optional<JWT> JWT on success, empty on error
        [[nodiscard]] static auto create() -> std::optional<JWT> {
            if (auto key = crypto::random::bytes(32))
                return JWT::create(*key);

            return {};
        }

        /// @brief Encode a JSON Web Token to a string
        /// @param token Token to encode
        /// @return std::optional<std::string> Encoded string on success, empty on failure
        [[nodiscard]] auto encode(const jwt::Token &token) -> std::optional<std::string> {
            if (auto h = crypto::base64url::encode(serialize(token.header))) {
                if (auto p = crypto::base64url::encode(serialize(token.payload))) {
                    auto enc = *h + "." + *p;
                    if (auto s = crypto::hmac::sign(enc, secret)) {
                        return enc + "." + *s;
                    }
                }
            }

            return {};
        }

        /// @brief Decode a JSON Web Token from an encoded Range
        /// @param src Encoded JWT Range to decode
        /// @return std::optional<jwt::Token> Decoded JSON Web Token on success, empty on failure
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

            token.header  = deserialize<Token::Header>(*dec_header);
            token.payload = deserialize<rfl::Generic::Object>(*dec_payload);

            return token;
        }
    };

}// namespace harbour::crypto::jwt