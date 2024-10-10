///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file map.hpp
/// @brief Contains the implementation details for Harbours cookie map type

#pragma once

#include <string>

#include <ankerl/unordered_dense.h>
#include <rfl.hpp>

namespace harbour::cookies {

    /// The internal map for key=value of a cookie
    using Map = ankerl::unordered_dense::map<std::string, std::string>;

}// namespace harbour::cookies

/// Allow our cookies::Map to be serialized/deserialized to json
namespace rfl {
    template<>
    struct Reflector<harbour::cookies::Map> {
        using ReflType = rfl::Object<std::string>;

        static harbour::cookies::Map to(const ReflType &obj) noexcept {
            harbour::cookies::Map map;
            for (const auto &[k, v]: obj)
                map[k] = v;
            return map;
        }

        static ReflType from(const harbour::cookies::Map &map) {
            ReflType obj;
            for (const auto &[k, v]: map)
                obj[k] = v;
            return obj;
        }
    };
}// namespace rfl
