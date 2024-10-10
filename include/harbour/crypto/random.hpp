///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file random.hpp
/// @brief Contains the implementation details for Harbours crypto randomness generating methods

#pragma once

#include <vector>
#include <optional>
#include <string>

#include <openssl/rand.h>

#include "../log/log.hpp"

namespace harbour::crypto::random {

    /// @brief Generate a vector of n random bytes
    /// @param n Number of random bytes to generate
    /// @return std::optional<std::vector<uint8_t>> Vector on success, empty on failure
    auto bytes(std::size_t n) -> std::optional<std::vector<std::uint8_t>> {
        std::vector<std::uint8_t> buffer(n);

        if (!RAND_bytes(buffer.data(), buffer.size())) {
            log::warn("RAND_bytes failed!");
            return {};
        }

        return buffer;
    }

    /// @brief Generate an n length string of random bytes
    /// @param n Number of random bytes to generate
    /// @return std::optional<std::string> String on success, empty on failure
    auto string(std::size_t n) -> std::optional<std::string> {
        std::string buffer(n, 0);

        if (!RAND_bytes(reinterpret_cast<std::uint8_t *>(buffer.data()), buffer.size())) {
            log::warn("RAND_bytes failed!");
            return {};
        }

        return buffer;
    }

}// namespace harbour::crypto::random