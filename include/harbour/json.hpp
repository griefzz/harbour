///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file json.hpp
/// @brief Contains the implementation details for the harbours json type

#pragma once

#include <string>
#include <type_traits>
#include <istream>

#include <rfl/json.hpp>
#include <rfl.hpp>

namespace harbour {

    /// @brief Concept to check if a type is Jsonable
    /// @tparam T Type to check
    template<typename T>
    concept Jsonable = requires(T v) {
        { rfl::json::write(v) } -> std::convertible_to<std::string>;
    };

    /// @brief Convenience type to create a json string from an object used by Response
    struct json {
        constexpr json(Jsonable auto &&v) : data(rfl::json::write(v)) {}
        std::string data;
    };

    /// @brief Convert a Jsonable object into a json string
    /// @param obj Object to convert to a json string
    /// @return Jsonable object as a json string
    constexpr auto serialize(Jsonable auto &&obj) -> std::string { return rfl::json::write(obj); }

    /// @brief Deserialize a json string into an object
    /// @tparam T Type to create from json string
    /// @param str Json string to deserialize
    /// @return Deserialized object of type T
    template<typename T>
    constexpr auto deserialize(auto &&str) { return rfl::json::read<T>(str).value(); }

    /// @brief Deserialize a json string_view into an object
    /// @tparam T Type to create from json string
    /// @param str Json string to deserialize
    /// @return Deserialized object of type T
    template<typename T>
    constexpr auto deserialize(const std::string_view str) { return rfl::json::read<T>(std::string(str)).value(); }

    /// @brief Deserialize a json istream into an object
    /// @tparam T Type to create from json string
    /// @param stream Json istream to deserialize
    /// @return Deserialized object of type T
    template<typename T>
    constexpr auto deserialize(std::istream &stream) { return rfl::json::read<T>(stream).value(); }

}// namespace harbour