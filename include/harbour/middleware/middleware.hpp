///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file middleware.hpp
/// @brief Contains the implementation of harbours Middleware structure
#pragma once

#include <vector>
#include <optional>

#include <asio/awaitable.hpp>

#include <harbour/ship.hpp>
#include <harbour/request/request.hpp>
#include <harbour/response/response.hpp>

#include "files.hpp"
#include "verbose.hpp"
#include "basicauth.hpp"

namespace harbour {

    /// @brief Middleware structure for handling Ships
    struct Middleware {
        /// @brief Accept a Ship as middleware and any number of ships for your middleware to act on
        /// @param middleware Ship to use as middleware
        /// @param ...ship Ships for your middleware to act on
        Middleware(detail::ShipConcept auto middleware, detail::ShipConcept auto... ship) : middleware(detail::make_ship(middleware)) {
            (ships.emplace_back(detail::make_ship(ship)), ...);
        }

        /// @brief Process the middleware chain
        /// @param req Request used in the chain
        /// @param resp Response used in the chain
        /// @return Response for middleware chain if valid, empty Response type otherwise
        auto operator()(const Request &req, Response &resp) -> asio::awaitable<std::optional<Response>> {
            for (auto &&ship: ships) {
                // Exit on the first instance of a valid Response from the middleware
                if (auto v = co_await detail::ShipHandler(req, resp, middleware)) {
                    resp = *v;
                    break;
                }

                // Exit on the first instance of a valid Response from our Ships
                if (auto v = co_await detail::ShipHandler(req, resp, ship)) {
                    resp = *v;
                    break;
                }
            }

            co_return resp;
        }

        detail::Ship middleware;        ///< Middleware Ship to use
        std::vector<detail::Ship> ships;///< Ships to execute middleware on
    };

}// namespace harbour