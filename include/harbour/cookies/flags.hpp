///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file flags.hpp
/// @brief Contains the implementation details for Harbours cookie flags

#pragma once

#include <optional>
#include <string>
#include <chrono>
#include <ctime>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/chrono.h>

namespace harbour::cookies {

    /// All possible types for a cookies SameSitePolicy
    /// https://developer.mozilla.org/en-US/docs/Web/HTTP/Cookies#controlling_third-party_cookies_with_samesite
    enum class SameSitePolicy {
        Strict,
        Lax,
        None
    };

    namespace detail {

        /// @brief Convert a SameSitePolicy enum to a string
        /// @param ssp The SameSitePolicy to convert
        /// @return The string representation of a SameSitePolicy
        auto SameSitePolicy_string(SameSitePolicy ssp) -> std::string_view {
            switch (ssp) {
                case SameSitePolicy::Strict:
                    return "Strict";
                case SameSitePolicy::Lax:
                    return "Lax";
                case SameSitePolicy::None:
                    return "None";
                default:
                    return "Invalid";
            }
        }

        /// @brief Convert a time_point to a GMT date string
        /// @param expires Date to convert
        /// @return std::string Converted date
        auto tp_to_gmt_string(std::chrono::system_clock::time_point expires) -> std::string {
            std::time_t time = std::chrono::system_clock::to_time_t(expires);
#if defined(_WIN32)
            std::tm gmt;
            gmtime_s(&gmt, &time);
            return fmt::format("; Expires={:%a, %d %m %Y %H:%M:%S} GMT", gmt);
#else
            std::tm *gmt = std::gmtime(&time);
            return fmt::format("; Expires={:%a, %d %m %Y %H:%M:%S} GMT", *gmt);
#endif
        }

    }// namespace detail

    /// @brief All possible flags that can be set for a cookie
    struct Flags {
        bool is_http_only{false};                                    ///< Enable HttpOnly
        bool is_secure{false};                                       ///< Enable Secure
        std::optional<std::chrono::system_clock::time_point> expires;///< Expires attribute
        std::optional<std::chrono::seconds> max_age;                 ///< Max-Age attribute
        std::optional<SameSitePolicy> policy;                        ///< Same Site Policy
        std::optional<std::string> path;                             ///< Path attribute
        std::optional<std::string> domain;                           ///< Domain attribute

        /// @brief Convert a cookie flags object to a string
        /// @return Cookie flags as a string
        [[nodiscard]] auto string() const -> std::string {
            std::string s;

            if (is_http_only) s += "; HttpOnly";
            if (is_secure) s += "; Secure";
            if (expires) s += detail::tp_to_gmt_string(*expires);
            if (max_age) s += fmt::format("; Max-Age={}", max_age->count());
            if (path) s += fmt::format("; Path={}", *path);
            if (domain) s += fmt::format("; Domain={}", *domain);
            if (policy) s += fmt::format("; SameSite={}", detail::SameSitePolicy_string(*policy));

            return s;
        }
    };

}// namespace harbour::cookies