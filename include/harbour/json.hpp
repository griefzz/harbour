///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file json.hpp
/// @brief Contains the implementation details for the harbours json type

#pragma once

#include <ranges>

#include <nlohmann/json.hpp>

#define HARBOUR_JSONABLE NLOHMANN_DEFINE_TYPE_INTRUSIVE

namespace harbour {
    namespace json {

        using json_t         = nlohmann::json;
        using ordered_json_t = nlohmann::ordered_json;

        /// @brief Concept to check if a type is Jsonable
        /// @tparam T Type to check
        template<typename T>
        concept Jsonable = requires(T v) {
            { nlohmann::json::parse(v) } -> std::convertible_to<json_t>;
            { nlohmann::ordered_json::parse(v) } -> std::convertible_to<ordered_json_t>;
        };

        /// @brief Convert a Jsonable object into a json_t
        /// @param obj Object to convert to json
        /// @return Converted json object
        auto serialize(Jsonable auto &&obj) -> json_t { return nlohmann::json::parse(obj); }

        /// @brief Convert a Jsonable object into an ordered_json_t preserving order
        /// @param obj Object to convert to json
        /// @return Converted json object
        auto serialize_ordered(Jsonable auto &&obj) -> ordered_json_t { return nlohmann::ordered_json::parse(obj); }

        /// @brief Deserialze a json object into a destination
        /// @param From Json object to deserialze
        /// @param To HARBOUR_JSONABLE object
        constexpr auto deserialize(Jsonable auto &&From, Jsonable auto &&To) { From.get_to(To); }

    }// namespace json
}// namespace harbour