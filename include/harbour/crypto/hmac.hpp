///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file hmac.hpp
/// @brief Contains the implementation details for Harbours crypto HMAC methods

#pragma once

#include <vector>
#include <cstdint>
#include <ranges>

#include <openssl/hmac.h>

#include "../ranges.hpp"

namespace harbour {
    namespace crypto {
        namespace hmac {

            using ranges::RandomAccessScalarRange;

            namespace detail {

                using u8     = std::uint8_t;
                using u32    = std::uint32_t;
                using s32    = std::int32_t;
                using size_t = std::size_t;

                auto constant_compare(RandomAccessScalarRange auto &&x,
                                      RandomAccessScalarRange auto &&y) -> bool {
                    if (x.size() != y.size())
                        return false;

                    u32 v = 0;
                    for (size_t i = 0; i < x.size(); i++)
                        v |= x[i] ^ y[i];

                    return s32((u32(v ^ 0) - 1) >> 31);
                }

            }// namespace detail

            /// @brief Sign data using HMAC with a private key
            /// @param data Data to sign
            /// @param key Private key for signing
            /// @return std::optional<std::string> Signed string on success, empty on failure
            auto sign(RandomAccessScalarRange auto &&data,
                      RandomAccessScalarRange auto &&key) -> std::optional<std::string> {
                using namespace detail;
                u32 len;
                std::string hash(EVP_MAX_MD_SIZE, 0);

                if (!HMAC(EVP_sha256(),
                          key.data(), key.size(),
                          reinterpret_cast<const u8 *>(data.data()), data.size(),
                          reinterpret_cast<u8 *>(hash.data()), &len))
                    return {};

                hash.resize(len);

                return hash;
            }

            /// @brief Verify the signature using HMAC of some data using the recieved mac and private key
            /// @param data Data to verify
            /// @param received_mac Recieved mac of the data
            /// @param key Private key for signing
            /// @return bool True on success, false on failure
            auto verify(RandomAccessScalarRange auto &&data,
                        RandomAccessScalarRange auto &&received_mac,
                        RandomAccessScalarRange auto &&key) -> bool {
                using namespace detail;
                if (auto computed_mac = sign(data, key))
                    return constant_compare(*computed_mac, received_mac);

                return false;
            }

        }// namespace hmac
    }// namespace crypto
}// namespace harbour