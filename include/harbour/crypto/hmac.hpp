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

namespace harbour::crypto::hmac::detail {

    auto constant_compare(ranges::RandomAccessScalarRange auto &&x,
                          ranges::RandomAccessScalarRange auto &&y) -> bool {
        if (x.size() != y.size())
            return false;

        std::uint32_t v = 0;
        for (std::size_t i = 0; i < x.size(); i++)
            v |= x[i] ^ y[i];

        return std::int32_t((std::uint32_t(v ^ 0) - 1) >> 31);
    }

}// namespace harbour::crypto::hmac::detail

namespace harbour::crypto::hmac {

    using ranges::RandomAccessScalarRange;

    /// @brief Sign data using HMAC with a private key
    /// @param data Data to sign
    /// @param key Private key for signing
    /// @return std::optional<std::string> Signed string on success, empty on failure
    auto sign(RandomAccessScalarRange auto &&data, RandomAccessScalarRange auto &&key) -> std::optional<std::string> {
        std::uint32_t len;
        std::string hash(EVP_MAX_MD_SIZE, 0);

        if (!HMAC(EVP_sha256(),
                  key.data(), key.size(),
                  reinterpret_cast<const std::uint8_t *>(data.data()), data.size(),
                  reinterpret_cast<std::uint8_t *>(hash.data()), &len))
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
        if (auto computed_mac = sign(data, key))
            return detail::constant_compare(*computed_mac, received_mac);

        return false;
    }

}// namespace harbour::crypto::hmac