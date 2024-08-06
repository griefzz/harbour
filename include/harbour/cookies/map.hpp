///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file map.hpp
/// @brief Contains the implementation details for Harbours cookie map type

#pragma once

#include <string>

#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>

#include <ankerl/unordered_dense.h>

namespace harbour {
    namespace cookies {

        /// The internal map for key=value of a cookie
        using Map = ankerl::unordered_dense::map<std::string, std::string>;

    }// namespace cookies
}// namespace harbour

/// @brief Allow our cookies::Map to be serialized/deserialized to json
namespace nlohmann {
    template<>
    struct adl_serializer<harbour::cookies::Map> {
        static void to_json(json &j, const harbour::cookies::Map &opt) {
            for (const auto &[k, v]: opt)
                j[k] = v;
        }

        static void from_json(const json &j, harbour::cookies::Map &opt) {
            for (const auto &item: j.items())
                opt[item.key()] = item.value();
        }

        static void to_json(ordered_json &j, const harbour::cookies::Map &opt) {
            for (const auto &[k, v]: opt)
                j[k] = v;
        }

        static void from_json(const ordered_json &j, harbour::cookies::Map &opt) {
            for (const auto &item: j.items())
                opt[item.key()] = item.value();
        }
    };
}// namespace nlohmann