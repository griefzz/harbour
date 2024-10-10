///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file base64url.hpp
/// @brief Contains the implementation details for Harbours crypto base64url methods

#pragma once

#include <optional>
#include <string>
#include <cmath>
#include <ranges>
#include <algorithm>

#include "base64.hpp"
#include "../ranges.hpp"


namespace harbour::crypto::base64url::detail {

    using ranges::RandomAccessScalarRange;

    /// @brief Convert a base64 character to base64url encoding
    /// @param ch Character to convert
    /// @return Converted character
    auto to_b64url(unsigned char ch) -> unsigned char {
        if (ch == '+') return '-';
        if (ch == '/') return '_';
        return ch;
    }

    /// @brief Convert a base64url character to base64 encoding
    /// @param ch Character to convert
    /// @return Converted character
    auto from_b64url(unsigned char ch) -> unsigned char {
        if (ch == '-') return '+';
        if (ch == '_') return '/';
        return ch;
    }

    /// @brief Count the number of padding characters in a base64url encoded string
    ///        We use this since base64url removes the '=' padding characters and
    ///        OpenSSL expects them for decoding.
    /// @param str Base64 range to count padding
    /// @return Number of trailing '='
    auto count_padding(RandomAccessScalarRange auto &&str) -> std::size_t {
        std::size_t n = 0;
        if (str.size() >= 2) {
            if (str[str.size() - 1] == '=') n++;
            if (str[str.size() - 2] == '=') n++;
        }
        return n;
    }

    /// @brief Get the number of padding characters required for the OpenSSL base64 decoder
    /// @param str Base64 range to get padding for
    /// @return number of trailing '=' needed
    auto num_padding_needed(RandomAccessScalarRange auto &&str) -> std::size_t {
        return (4ULL - (str.size() % 4ULL)) % 4ULL;
    }

}// namespace harbour::crypto::base64url::detail


namespace harbour::crypto::base64url {

    using ranges::RandomAccessScalarRange;

    /// @brief Encode src into a base64url string
    /// @param src Source to encode
    /// @return std::optional<std::string> Encoded string on success or empty on error
    auto encode(RandomAccessScalarRange auto &&src) -> std::optional<std::string> {
        using namespace detail;

        if (auto enc = crypto::base64::encode(src)) {
            // convert to url compatible encoding
            std::ranges::transform(enc->begin(), enc->end(), enc->begin(), to_b64url);

            // remove padding characters
            enc->resize(enc->size() - count_padding(*enc));

            return enc;
        }

        return {};
    }

    /// @brief Decode src from a base64url string
    /// @param src Source to encode
    /// @return std::optional<std::string> Decoded string on success or empty on error
    auto decode(RandomAccessScalarRange auto &&src) -> std::optional<std::string> {
        using namespace detail;

        if (src.size() == 0)
            return {};

        // Convert to base64 encoding from base64url
        auto b = src;
        std::ranges::transform(b.begin(), b.end(), b.begin(), from_b64url);

        // Add the padding characters OpenSSL expects
        for (std::size_t i = 0; i < num_padding_needed(src); i++)
            b += '=';

        // Decode from base64
        return crypto::base64::decode(b);
    }

}// namespace harbour::crypto::base64url