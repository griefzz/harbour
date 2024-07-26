///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file ship.hpp
/// @brief Contains the implementation details for the harbour Ship type

#pragma once

#include <functional>
#include <variant>
#include <concepts>
#include <optional>

#include <asio/awaitable.hpp>

#include "request/request.hpp"
#include "response/response.hpp"

namespace harbour {
    namespace detail {

        using asio::awaitable;

        /// @brief Trait to determine if a type is awaitable.
        /// @tparam T The type to check.
        template<typename T>
        struct is_awaitable : std::false_type {};

        /// @brief Specialization of is_awaitable for types that are awaitable.
        /// @tparam T The type to check.
        template<typename T>
        struct is_awaitable<awaitable<T>> : std::true_type {};

        /// @brief Concept to check if a type T is any of the types U.
        /// @tparam T The type to check.
        /// @tparam U The types to check against.
        template<typename T, typename... U>
        concept is_any_of = (std::same_as<T, U> || ...);

        /// @brief Helper struct to overload multiple function call operators.
        /// @tparam Ts The types of the function call operators.
        template<class... Ts>
        struct overloaded : Ts... {
            using Ts::operator()...;
        };

        /// @brief Type aliases for various function signatures.
        using Ship_0  = std::function<awaitable<Response>(const Request &, Response &)>;
        using Ship_1  = std::function<awaitable<Response>(Response &, const Request &)>;
        using Ship_2  = std::function<awaitable<std::optional<Response>>(const Request &, Response &)>;
        using Ship_3  = std::function<awaitable<std::optional<Response>>(Response &, const Request &)>;
        using Ship_4  = std::function<awaitable<void>(const Request &, Response &)>;
        using Ship_5  = std::function<awaitable<void>(Response &, const Request &)>;
        using Ship_6  = std::function<awaitable<Response>(const Request &)>;
        using Ship_7  = std::function<awaitable<Response>(Response &)>;
        using Ship_8  = std::function<awaitable<std::optional<Response>>(const Request &)>;
        using Ship_9  = std::function<awaitable<std::optional<Response>>(Response &)>;
        using Ship_10 = std::function<awaitable<void>(const Request &)>;
        using Ship_11 = std::function<awaitable<void>(Response &)>;
        using Ship_12 = std::function<awaitable<Response>()>;
        using Ship_13 = std::function<awaitable<std::optional<Response>>()>;
        using Ship_14 = std::function<awaitable<void>()>;
        /// ===========================================================================================
        using Ship_15 = std::function<Response(const Request &, Response &)>;
        using Ship_16 = std::function<Response(Response &, const Request &)>;
        using Ship_17 = std::function<std::optional<Response>(const Request &, Response &)>;
        using Ship_18 = std::function<std::optional<Response>(Response &, const Request &)>;
        using Ship_19 = std::function<void(const Request &, Response &)>;
        using Ship_20 = std::function<void(Response &, const Request &)>;
        using Ship_21 = std::function<Response(const Request &)>;
        using Ship_22 = std::function<Response(Response &)>;
        using Ship_23 = std::function<std::optional<Response>(const Request &)>;
        using Ship_24 = std::function<std::optional<Response>(Response &)>;
        using Ship_25 = std::function<void(const Request &)>;
        using Ship_26 = std::function<void(Response &)>;
        using Ship_27 = std::function<Response()>;
        using Ship_28 = std::function<std::optional<Response>()>;
        using Ship_29 = std::function<void()>;

        /// @brief Main ship variant used
        using Ship = std::variant<Ship_0, Ship_1, Ship_2, Ship_3, Ship_4,
                                  Ship_5, Ship_6, Ship_7, Ship_8, Ship_9,
                                  Ship_10, Ship_11, Ship_12, Ship_13, Ship_14,
                                  Ship_15, Ship_16, Ship_17, Ship_18,
                                  Ship_19, Ship_20, Ship_21, Ship_22,
                                  Ship_23, Ship_24, Ship_25, Ship_26,
                                  Ship_27, Ship_28, Ship_29>;

        /// @brief Concept to check if a type T is a ship.
        /// @tparam T The type to check.
        template<typename T>
        concept is_ship = is_any_of<std::remove_cvref_t<std::remove_pointer_t<std::decay_t<T>>>,
                                    Ship_0, Ship_1, Ship_2, Ship_3, Ship_4,
                                    Ship_5, Ship_6, Ship_7, Ship_8, Ship_9,
                                    Ship_10, Ship_11, Ship_12, Ship_13, Ship_14,
                                    Ship_15, Ship_16, Ship_17, Ship_18,
                                    Ship_19, Ship_20, Ship_21, Ship_22,
                                    Ship_23, Ship_24, Ship_25, Ship_26,
                                    Ship_27, Ship_28, Ship_29>;

        /// @brief Concept to check if a type T satisfies the ShipConcept.
        /// @tparam T The type to check.
        template<typename T>
        concept ShipConcept =
                (is_ship<T> ||
                 std::is_invocable_r_v<awaitable<Response>, T, const Request &, Response &> ||
                 std::is_invocable_r_v<awaitable<Response>, T, Response &, const Request &> ||
                 std::is_invocable_r_v<awaitable<std::optional<Response>>, T, const Request &, Response &> ||
                 std::is_invocable_r_v<awaitable<std::optional<Response>>, T, Response &, const Request &> ||
                 std::is_invocable_r_v<awaitable<void>, T, const Request &, Response &> ||
                 std::is_invocable_r_v<awaitable<void>, T, Response &, const Request &> ||
                 std::is_invocable_r_v<awaitable<Response>, T, const Request &> ||
                 std::is_invocable_r_v<awaitable<Response>, T, Response &> ||
                 std::is_invocable_r_v<awaitable<std::optional<Response>>, T, const Request &> ||
                 std::is_invocable_r_v<awaitable<std::optional<Response>>, T, Response &> ||
                 std::is_invocable_r_v<awaitable<void>, T, const Request &> ||
                 std::is_invocable_r_v<awaitable<void>, T, Response &> ||
                 std::is_invocable_r_v<awaitable<Response>, T> ||
                 std::is_invocable_r_v<awaitable<std::optional<Response>>, T> ||
                 std::is_invocable_r_v<awaitable<void>, T> ||
                 std::is_invocable_r_v<Response, T, const Request &, Response &> ||
                 std::is_invocable_r_v<Response, T, Response &, const Request &> ||
                 std::is_invocable_r_v<std::optional<Response>, T, const Request &, Response &> ||
                 std::is_invocable_r_v<std::optional<Response>, T, Response &, const Request &> ||
                 std::is_invocable_r_v<void, T, const Request &, Response &> ||
                 std::is_invocable_r_v<void, T, Response &, const Request &> ||
                 std::is_invocable_r_v<Response, T, const Request &> ||
                 std::is_invocable_r_v<Response, T, Response &> ||
                 std::is_invocable_r_v<std::optional<Response>, T, const Request &> ||
                 std::is_invocable_r_v<std::optional<Response>, T, Response &> ||
                 std::is_invocable_r_v<void, T, const Request &> ||
                 std::is_invocable_r_v<void, T, Response &> ||
                 std::is_invocable_r_v<Response, T> ||
                 std::is_invocable_r_v<std::optional<Response>, T> ||
                 std::is_invocable_r_v<void, T>);

        /// @brief Creates a Ship from a given ShipConcept.
        /// @tparam S The type of the ship concept.
        /// @param s The ship concept instance.
        /// @return A Ship variant.
        constexpr auto make_ship(ShipConcept auto &&s) {
            using S = decltype(s);
            using T = std::decay_t<S>;

            if constexpr (is_ship<T>)
                return Ship{s};
            else if constexpr (std::is_invocable_r_v<awaitable<Response>, S, const Request &, Response &>)
                return Ship{Ship_0{s}};
            else if constexpr (std::is_invocable_r_v<awaitable<Response>, S, Response &, const Request &>)
                return Ship{Ship_1{s}};
            else if constexpr (std::is_invocable_r_v<awaitable<std::optional<Response>>, S, const Request &, Response &>)
                return Ship{Ship_2{s}};
            else if constexpr (std::is_invocable_r_v<awaitable<std::optional<Response>>, S, Response &, const Request &>)
                return Ship{Ship_3{s}};
            else if constexpr (std::is_invocable_r_v<awaitable<void>, S, const Request &, Response &>)
                return Ship{Ship_4{s}};
            else if constexpr (std::is_invocable_r_v<awaitable<void>, S, Response &, const Request &>)
                return Ship{Ship_5{s}};
            else if constexpr (std::is_invocable_r_v<awaitable<Response>, S, const Request &>)
                return Ship{Ship_6{s}};
            else if constexpr (std::is_invocable_r_v<awaitable<Response>, S, Response &>)
                return Ship{Ship_7{s}};
            else if constexpr (std::is_invocable_r_v<awaitable<std::optional<Response>>, S, const Request &>)
                return Ship{Ship_8{s}};
            else if constexpr (std::is_invocable_r_v<awaitable<std::optional<Response>>, S, Response &>)
                return Ship{Ship_9{s}};
            else if constexpr (std::is_invocable_r_v<awaitable<void>, S, const Request &>)
                return Ship{Ship_10{s}};
            else if constexpr (std::is_invocable_r_v<awaitable<void>, S, Response &>)
                return Ship{Ship_11{s}};
            else if constexpr (std::is_invocable_r_v<awaitable<Response>, S>)
                return Ship{Ship_12{s}};
            else if constexpr (std::is_invocable_r_v<awaitable<std::optional<Response>>, S>)
                return Ship{Ship_13{s}};
            else if constexpr (std::is_invocable_r_v<awaitable<void>, S>)
                return Ship{Ship_14{s}};
            /// =======================================================================================================
            else if constexpr (std::is_invocable_r_v<Response, S, const Request &, Response &>)
                return Ship{Ship_15{s}};
            else if constexpr (std::is_invocable_r_v<Response, S, Response &, const Request &>)
                return Ship{Ship_16{s}};
            else if constexpr (std::is_invocable_r_v<std::optional<Response>, S, const Request &, Response &>)
                return Ship{Ship_17{s}};
            else if constexpr (std::is_invocable_r_v<std::optional<Response>, S, Response &, const Request &>)
                return Ship{Ship_18{s}};
            else if constexpr (std::is_invocable_r_v<void, S, const Request &, Response &>)
                return Ship{Ship_19{s}};
            else if constexpr (std::is_invocable_r_v<void, S, Response &, const Request &>)
                return Ship{Ship_20{s}};
            else if constexpr (std::is_invocable_r_v<Response, S, const Request &>)
                return Ship{Ship_21{s}};
            else if constexpr (std::is_invocable_r_v<Response, S, Response &>)
                return Ship{Ship_22{s}};
            else if constexpr (std::is_invocable_r_v<std::optional<Response>, S, const Request &>)
                return Ship{Ship_23{s}};
            else if constexpr (std::is_invocable_r_v<std::optional<Response>, S, Response &>)
                return Ship{Ship_24{s}};
            else if constexpr (std::is_invocable_r_v<void, S, const Request &>)
                return Ship{Ship_25{s}};
            else if constexpr (std::is_invocable_r_v<void, S, Response &>)
                return Ship{Ship_26{s}};
            else if constexpr (std::is_invocable_r_v<Response, S>)
                return Ship{Ship_27{s}};
            else if constexpr (std::is_invocable_r_v<std::optional<Response>, S>)
                return Ship{Ship_28{s}};
            else if constexpr (std::is_invocable_r_v<void, S>)
                return Ship{Ship_29{s}};
        }

        /// @brief Handler for processing a Request and Response using a Ship.
        constexpr auto ShipHandler = [](const Request &req, Response &resp, auto &&ship) -> awaitable<std::optional<Response>> {
            auto handle = [&](auto &&s, auto &&...args) -> awaitable<std::optional<Response>> {
                using ReturnType = decltype(s(args...));

                if constexpr (is_awaitable<ReturnType>::value) {
                    if constexpr (std::is_same_v<awaitable<void>, decltype(s(args...))>) {
                        co_await s(args...);
                        co_return std::nullopt;
                    } else {
                        co_return co_await s(args...);
                    }
                } else {
                    if constexpr (std::is_same_v<void, decltype(s(args...))>) {
                        s(args...);
                        co_return std::nullopt;
                    } else {
                        auto ret = s(args...);
                        co_return ret;
                    }
                }
            };

            auto ol = detail::overloaded{
                    [&](auto &s) -> awaitable<std::optional<Response>> {
                        if constexpr (requires { s(req, resp); }) {
                            co_return co_await handle(s, req, resp);
                        } else if constexpr (requires { s(resp, req); }) {
                            co_return co_await handle(s, resp, req);
                        } else if constexpr (requires { s(req); }) {
                            co_return co_await handle(s, req);
                        } else if constexpr (requires { s(resp); }) {
                            co_return co_await handle(s, resp);
                        } else {
                            co_return co_await handle(s);
                        }
                    }};

            co_return co_await std::visit(ol, ship);
        };

    }// namespace detail
}// namespace harbour