///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file sha.hpp
/// @brief Contains the implementation details for Harbours crypto sha methods

#pragma once

#include <string>
#include <ranges>

#include <openssl/sha.h>

#include "../ranges.hpp"

namespace harbour {
    namespace crypto {
        namespace sha {

            using ranges::RandomAccessScalarRange;

            /// @brief Create a SHA1 hash from plaintext
            /// @param plaintext Data to hash
            /// @return std::string SHA1 hashed data
            auto sha1(RandomAccessScalarRange auto &&plaintext) -> std::string {
                std::string hash(SHA_DIGEST_LENGTH, 0);
                SHA1(reinterpret_cast<const unsigned char *>(plaintext.data()), plaintext.size(), reinterpret_cast<unsigned char *>(hash.data()));
                return hash;
            }

        }// namespace sha
    }// namespace crypto
}// namespace harbour