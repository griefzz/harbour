///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file headers.hpp
/// @brief Contains the implementation details for the harbours Response headers type

#pragma once

#include <string>

#include <ankerl/unordered_dense.h>

#include <fmt/base.h>
#include <fmt/format.h>

namespace harbour::response {

    /// @brief Header map containing key/values for Response
    using Headers = ankerl::unordered_dense::map<std::string, std::string>;

}// namespace harbour::response

/// @brief Allow Headers to be formatted using fmtlib
template<>
struct fmt::formatter<harbour::response::Headers> : formatter<string_view> {
    auto format(const harbour::response::Headers &map, format_context &ctx) const -> format_context::iterator {
        std::string s;
        for (const auto &[k, v]: map)
            s += fmt::format("{}: {}\n", k, v);

        return formatter<string_view>::format(s, ctx);
    }
};