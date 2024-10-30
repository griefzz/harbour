///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file securecookies.hpp
/// @brief Contains the implementation details for Harbours secure cookies handler

#pragma once

#include <array>
#include <cstdint>
#include <chrono>
#include <ranges>
#include <string>
#include <charconv>
#include <system_error>

#include "cookies.hpp"
#include "../ranges.hpp"
#include "../request/request.hpp"
#include "../crypto/aes.hpp"
#include "../crypto/base64.hpp"
#include "../crypto/hmac.hpp"
#include "../crypto/random.hpp"
#include "../json.hpp"

namespace harbour {
    namespace cookies::detail {

        /// @brief Generate a Unix UTC timestamp for the current time
        /// @return size_t Unix UTC timestamp
        auto timestamp() -> std::size_t {
            return std::chrono::time_point_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now())
                    .time_since_epoch()
                    .count();
        }
        /// @brief Convert a date string to a type T
        /// @tparam T Type to convert string to
        /// @param date Date to convert
        /// @return T Converted date string
        template<typename T>
        auto date_to_v(ranges::RandomAccessScalarRange auto &&date) -> T {
            T t{};
            std::from_chars(date.data(), date.data() + date.size(), t);
            return t;
        }

    }// namespace cookies::detail

    struct SecureCookies {
        Cookies cookies;
        std::array<std::uint8_t, 32> hash_key;
        std::array<std::uint8_t, 32> block_key;
        std::size_t max_length{4096};

        /// @brief Create a SecureCookies object from a given hash key and block key
        /// @param HashKey 32-byte Hash key used for HMAC authentication
        /// @param BlockKey 32-byte Block key used for AES256-CTR encryption
        /// @return std::optional<SecureCookies> SecureCookies on success, empty on failure
        [[nodiscard]] static auto create(ranges::RandomAccessCharRange auto &&HashKey,
                                         ranges::RandomAccessCharRange auto &&BlockKey) -> std::optional<SecureCookies> {
            using namespace std::chrono_literals;
            if (HashKey.size() != 32) return {};
            if (BlockKey.size() != 32) return {};
            SecureCookies sc;
            (void) sc.cookies.with_age(2'592'000s);// 86400 * 30

            auto hash_ptr  = reinterpret_cast<const std::uint8_t *>(HashKey.data());
            auto block_ptr = reinterpret_cast<const std::uint8_t *>(BlockKey.data());

            std::copy(hash_ptr, hash_ptr + HashKey.size(), sc.hash_key.begin());
            std::copy(block_ptr, block_ptr + BlockKey.size(), sc.block_key.begin());
            return sc;
        }

        /// @brief Create a SecureCookies object with a randomly generated hash key and block key
        /// @return std::optional<SecureCookies> SecureCookies on success, empty on failure
        [[nodiscard]] static auto create() -> std::optional<SecureCookies> {
            auto hash  = crypto::random::bytes(32);
            auto block = crypto::random::bytes(32);
            if (!hash || !block)
                return {};

            return SecureCookies::create(*hash, *block);
        }

        /// @brief Encode a cookie value with the given name into a secure cookie string
        /// @param name Name of the cookie value
        /// @param value Value of the cookie
        /// @return Encoded secure cookie string on success, empty on failure
        auto encode(const std::string &name, Jsonable auto &&value) -> std::string {
            // 1. Serialize
            auto b = serialize(value);

            // 2. Encrypt
            auto block = std::string(block_key.begin(), block_key.end());
            auto hash  = std::string(hash_key.begin(), hash_key.end());

            b = crypto::aes256::encrypt(b, block).value_or("");
            if (b.empty())
                return {};

            b = crypto::base64::encode(b).value_or("");
            if (b.empty())
                return {};

            // 3. Create MAC for "name|date|value". Extra pipe to be used later.
            b        = fmt::format("{}|{}|{}|", name, cookies::detail::timestamp(), b);
            auto sig = std::string_view(b.begin(), b.end() - 1);
            auto mac = crypto::hmac::sign(sig, hash).value_or("");
            if (mac.empty())
                return {};

            // Append mac, remove name.
            b += mac;
            b = std::string(b.begin() + name.size() + 1, b.end());

            // 4. Encode to base64.
            b = crypto::base64::encode(b).value_or("");
            if (b.empty())
                return {};

            // 5. Check length.
            if (max_length != 0 && b.size() > max_length)
                return {};

            return b;
        }

        /// @brief Decode a secure cookie object into a value
        /// @param name Name of the secure cookie value
        /// @param data Secure cookie data
        /// @param value Value to decode the result into
        /// @return bool True on success, false on failure
        auto decode(const std::string &name, const std::string &data, Jsonable auto &&value) -> bool {
            if (name.empty())
                return false;

            // 1. Check length.
            if (max_length != 0 && data.size() > max_length)
                return false;

            // 2. Decode from base64.
            auto b = crypto::base64::decode(data).value_or("");
            if (b.empty())
                return false;

            // 3. Verify MAC. Value is "date|value|mac".
            // break the cookie into 3 parts
            auto parts = b | std::views::split('|') | std::views::take(3) | ranges::to<std::vector>;
            if (parts.size() != 3) return false;

            auto date = std::string(parts[0].begin(), parts[0].end());// Date
            auto val  = std::string(parts[1].begin(), parts[1].end());// Encoded value
            auto mac  = std::string(parts[2].begin(), parts[2].end());// MAC

            b = name + "|" + std::string(b.begin(), b.begin() + b.size() - mac.size() - 1);

            auto hash = std::string(hash_key.begin(), hash_key.end());
            auto ok   = crypto::hmac::verify(b, mac, hash);
            if (!ok)
                return false;

            // 4. Verify date ranges.
            auto t1 = cookies::detail::date_to_v<std::size_t>(date);
            auto t2 = cookies::detail::timestamp();
            if (cookies.flags.max_age && cookies.flags.max_age->count() != 0 && t1 < t2 - cookies.flags.max_age->count())
                return false;

            // 5. Decrypt.
            b = crypto::base64::decode(val).value_or("");
            if (b.empty())
                return false;

            auto block = std::string(block_key.begin(), block_key.end());
            b          = crypto::aes256::decrypt(b, block).value_or("");
            if (b.empty())
                return false;

            // 6. Deserialize.
            try {
                value = deserialize<std::decay_t<decltype(value)>>(b);
            } catch (...) {
                log::warn("Failed to deserialze object in securecookies");
                return false;
            }

            return true;
        }
    };

}// namespace harbour
