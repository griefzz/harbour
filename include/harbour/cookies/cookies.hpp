///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file cookies.hpp
/// @brief Contains the implementation details for Harbours cookies handler

#pragma once

#include <ranges>
#include <string_view>
#include <string>
#include <concepts>
#include <chrono>
#include <optional>
#include <algorithm>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/chrono.h>
#include <fmt/base.h>

#include "flags.hpp"
#include "parser.hpp"
#include "map.hpp"
#include "../ranges.hpp"
#include "../request/request.hpp"
#include "../response/response.hpp"

namespace harbour {

    struct Cookies {
        cookies::Map data;   ///< Internal cookie map
        cookies::Flags flags;///< Internal cookie flags

        /// @brief Creates an empty Cookies object.
        /// @return An empty Cookies object.
        [[nodiscard]] static auto create() -> Cookies { return Cookies{}; }

        /// @brief Creates a Cookies object from a Request.
        /// @param r The Request object.
        /// @return An optional Cookies object if creation is successful.
        [[nodiscard]] static auto create(const Request &r) -> std::optional<Cookies> {
            if (const auto v = r.header("Cookie")) {
                if (const auto cookie = cookies::detail::parse(*v)) {
                    const auto &[data, flags] = *cookie;
                    return Cookies{data, flags};
                }
            }

            return {};
        }

        /// @brief Creates a Cookies object from a cookie string.
        /// @param r The cookie string.
        /// @return An optional Cookies object if creation is successful.
        [[nodiscard]] static auto create(ranges::RandomAccessScalarRange auto &&r) -> std::optional<Cookies> {
            if (auto cookie = cookies::detail::parse(r))
                return Cookies{*cookie};

            return {};
        }

        /// @brief Set the Expires attribute of the cookie to a certain date
        /// @see https://developer.mozilla.org/en-US/docs/Web/HTTP/Cookies#removal_defining_the_lifetime_of_a_cookie
        /// @param date The date the cookie expires
        /// @return Reference to the modified Cookies object.
        [[nodiscard]] auto with_expires(const std::chrono::system_clock::time_point &date) -> Cookies & {
            flags.expires = date;
            return *this;
        }

        /// @brief Set the Max-Age attribute of a cookie
        /// @see https://developer.mozilla.org/en-US/docs/Web/HTTP/Cookies#removal_defining_the_lifetime_of_a_cookie
        /// @param age The max age of the cookie in seconds
        /// @return Reference to the modified Cookies object.
        [[nodiscard]] auto with_age(const std::chrono::seconds &age) -> Cookies & {
            flags.max_age = age;
            return *this;
        }

        /// @brief Set the Max-Age attribute of a cookie
        /// @see https://developer.mozilla.org/en-US/docs/Web/HTTP/Cookies#removal_defining_the_lifetime_of_a_cookie
        /// @param age The max age of the cookie in seconds
        /// @return Reference to the modified Cookies object.
        [[nodiscard]] auto with_age(std::size_t age) -> Cookies & {
            flags.max_age = std::chrono::seconds(age);
            return *this;
        }

        /// @brief Set the Domain attribute of a cookie
        /// @see https://developer.mozilla.org/en-US/docs/Web/HTTP/Cookies#define_where_cookies_are_sent
        /// @param domain The valid domain of a cookie
        /// @return Reference to the modified Cookies object.
        [[nodiscard]] auto with_domain(auto &&domain) -> Cookies & {
            flags.domain = domain;
            return *this;
        }
        /// @brief Set the Path attribute of a cookie
        /// @see https://developer.mozilla.org/en-US/docs/Web/HTTP/Cookies#define_where_cookies_are_sent
        /// @param path The valid Path of a cookie
        /// @return Reference to the modified Cookies object.
        [[nodiscard]] auto with_path(auto &&path) -> Cookies & {
            flags.path = path;
            return *this;
        }

        /// @brief Set the SameSite attribute of a cookie
        /// @see https://developer.mozilla.org/en-US/docs/Web/HTTP/Cookies#controlling_third-party_cookies_with_samesite
        /// @param policy The Same Site Policy of the cookie
        /// @return Reference to the modified Cookies object.
        [[nodiscard]] auto with_same_site_policy(cookies::SameSitePolicy policy) -> Cookies & {
            flags.policy = policy;
            return *this;
        }

        /// @brief Enable the Secure flag for a cookie
        /// @see https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Set-Cookie#secure
        /// @return Reference to the modified Cookies object.
        [[nodiscard]] auto with_secure() -> Cookies & {
            flags.is_secure = true;
            return *this;
        }

        /// @brief Enable the HttpOnly flag for a cookie
        /// @see https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Set-Cookie#httponly
        /// @return Reference to the modified Cookies object.
        [[nodiscard]] auto with_http_only() -> Cookies & {
            flags.is_http_only = true;
            return *this;
        }

        /// @brief Set a Key=Value in the cookie map
        /// @param Key Key for the Value
        /// @param Value Value for the Key
        auto set(auto &&Key, auto &&Value) -> void {
            data[Key] = Value;
        }

        /// @brief Get a Value for a Key=Value entry in the cookie map.
        ///        Returns empty if the Key is not found
        /// @param Key Key to get
        /// @return Value for Key. Empty if Key doesnt exist in the cookie map.
        [[nodiscard]] auto get(auto &&Key) -> std::optional<std::string> {
            if (auto it = data.find(Key); it != data.end()) {
                return it->second;
            }
            return {};
        }

        /// @brief Access header value by key.
        //         This operation will automatically create an entry for the map.
        /// @param Key Header key.
        /// @return Reference to the header value.
        [[nodiscard]] constexpr auto operator[](auto &&Key) -> std::string & {
            return data[Key];
        }

        /// @brief Converts the cookies to a string representation.
        /// @return A string representation of the cookies.
        [[nodiscard]] auto string() const -> std::string {
            // Assemble cookie data
            auto kv_pairs = data | std::views::transform([](const auto &pair) {
                                return pair.first + "=" + pair.second;
                            });

            auto s = fmt::format("{}", fmt::join(kv_pairs, "; "));

            // Assemble flags
            s += flags.string();

            return s;
        }
    };

}// namespace harbour

/// @brief Allow Cookies to be formatted using fmtlib
template<>
struct fmt::formatter<harbour::Cookies> : formatter<string_view> {
    auto format(const harbour::Cookies &cookie, format_context &ctx) const -> format_context::iterator {
        return formatter<string_view>::format(cookie.string(), ctx);
    }
};
