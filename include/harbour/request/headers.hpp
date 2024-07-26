///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file parser.hpp
/// @brief Contains the implementation of harbours Request headers

#pragma once

#include <string_view>

#include <ankerl/unordered_dense.h>

#include <fmt/base.h>
#include <fmt/format.h>

namespace harbour {
    namespace request {

        /// @brief @brief Constant Header map containing key/values for Request
        using Headers = ankerl::unordered_dense::map<std::string_view, std::string_view>;

    }// namespace request
}// namespace harbour

/// @brief Allow RequestHeaders to be formatted using fmtlib
template<>
struct fmt::formatter<harbour::request::Headers> : formatter<string_view> {
    auto format(const harbour::request::Headers &map, format_context &ctx) const -> format_context::iterator {
        std::string s;
        for (const auto &[k, v]: map)
            s += fmt::format("{}: {}\n", k, v);

        return formatter<string_view>::format(s, ctx);
    }
};