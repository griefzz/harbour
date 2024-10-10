///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file base64.hpp
/// @brief Contains the implementation details for Harbours crypto base64 methods

#pragma once

#include <optional>
#include <string>
#include <cmath>

#include <openssl/evp.h>

#include "../ranges.hpp"

namespace harbour::crypto::base64 {

    using ranges::RandomAccessScalarRange;

    /// @brief Encode src into a base64 string
    /// @param src Source to encode
    /// @return std::optional<std::string> Encoded string on success or empty on error
    auto encode(RandomAccessScalarRange auto &&src) -> std::optional<std::string> {
        if (src.size() == 0)
            return {};

        const std::size_t pl = 4 * std::ceil(static_cast<double>(src.size()) / 3);
        std::string out(pl + 1, 0);

        const auto ol = EVP_EncodeBlock(
                reinterpret_cast<unsigned char *>(out.data()),
                reinterpret_cast<const unsigned char *>(src.data()),
                src.size());

        if (pl != ol)
            return {};

        out.resize(ol);// remove null terminator

        return out;
    }

    /// @brief Decode src from a base64 string
    /// @param src Source to encode
    /// @return std::optional<std::string> Decoded string on success or empty on error
    auto decode(RandomAccessScalarRange auto &&src) -> std::optional<std::string> {
        if (src.size() == 0 || src.size() % 4 != 0)
            return {};

        const auto pl = 3 * src.size() / 4;
        std::string out(pl + 1, 0);

        auto ol = EVP_DecodeBlock(
                reinterpret_cast<unsigned char *>(out.data()),
                reinterpret_cast<const unsigned char *>(src.data()),
                src.size());

        if (pl != ol)
            return {};

        // detect and trim padding
        if (src[src.size() - 1] == '=') ol--;
        if (src[src.size() - 2] == '=') ol--;

        out.resize(ol);// resize accounting for padding

        return out;
    }

}// namespace harbour::crypto::base64