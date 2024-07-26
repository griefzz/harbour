///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file parser.hpp
/// @brief Contains the implementation details for Harbours cookie parser

#pragma once

#include <ranges>
#include <string>
#include <optional>
#include <utility>

#include "map.hpp"
#include "flags.hpp"
#include "../ranges.hpp"

namespace harbour {
    namespace cookies {
        namespace detail {

            /// @brief Parses a cookie string into a CookieData map.
            /// @param cookie The cookie string to parse.
            /// @return An optional CookieData map if parsing is successful.
            [[nodiscard]] auto parse(ranges::RandomAccessScalarRange auto &&cookie) -> std::optional<std::pair<Map, Flags>> {
                const std::string_view cookie_sep = ";";// Seperator for each cookie value entry
                const std::string_view kv_sep     = "=";// Seperator for each key=value entry of a cookie value
                Map data;
                Flags flags;

                // break the cookies into its 'key=value' or 'flag' components
                auto key_value_pairs = cookie | std::views::split(';') | ranges::to<std::vector>;

                // Parse the range of 'key=value' or 'flag'
                for (const auto &pair: key_value_pairs) {
                    if (pair.empty()) return {};// ; ; is a parsing error

                    // break the 'key=value' or 'flag' component into {key, value} or {flag}
                    auto it = std::ranges::find_first_of(pair, "=");

                    // Flag
                    if (it == pair.end()) {
                        auto flag = std::string(pair.begin(), pair.end());

                        // Trim leading and trailing spaces
                        flag.erase(0, flag.find_first_not_of(' '));
                        flag.erase(flag.find_last_not_of(' ') + 1);

                        if (flag == "HttpOnly")
                            flags.is_http_only = true;
                        else if (flag == "Secure")
                            flags.is_secure = true;
                        else// If the flag wasnt a known flag consider this a parsing failure
                            return {};
                    } else {// key=value pair
                        auto key   = std::string(pair.begin(), it);
                        auto value = std::string(it + 1, pair.end());

                        // Trim leading and trailing spaces
                        key.erase(0, key.find_first_not_of(' '));
                        key.erase(key.find_last_not_of(' ') + 1);
                        value.erase(0, value.find_first_not_of(' '));
                        value.erase(value.find_last_not_of(' ') + 1);

                        data[key] = value;
                    }
                }

                return std::make_pair(data, flags);
            }

        }// namespace detail
    }// namespace cookies
}// namespace harbour