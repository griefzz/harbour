///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file method.hpp
/// @brief Contains the implementation of harbours http method type

#pragma once

namespace harbour::http {

    /// @brief HTTP Request Method
    enum class Method : std::uint8_t {
        GET  = 1,     ///< GET method
        POST = 1 << 1,///< POST method
        PUT  = 1 << 2,///< PUT method (todo)
        HEAD = 1 << 3 ///< HEAD method (todo)
    };

    /// @brief Method constraint to allow multiple methods to be used
    using MethodConstraint = std::uint8_t;

    /// @brief Allow you to chain Methods into a MethodConstraint using |
    /// @param lhs Left Method to chain
    /// @param rhs Right Method to chain
    /// @return MethodContraint Constraint result from chain
    inline Method operator|(Method lhs, Method rhs) {
        using T = std::underlying_type_t<Method>;
        return static_cast<Method>(static_cast<T>(lhs) | static_cast<T>(rhs));
    }

    namespace detail {

        /// @brief Check if a Method matches a MethodConstraint
        /// @param mc MethodConstraint to use for Request Method
        /// @param m Request Method to test
        /// @return bool True if Method matches the constraint, false otherwise
        [[nodiscard]] inline auto matches_constraint(MethodConstraint mc, Method m) -> bool {
            return static_cast<MethodConstraint>(m) & mc;
        }

    }// namespace detail

}// namespace harbour::http