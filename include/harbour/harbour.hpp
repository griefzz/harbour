///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file harbour.hpp
/// @brief Contains the implementation details for Harbour

#pragma once

#include <iostream>
#include <vector>

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>

#include <fmt/core.h>
#include <fmt/color.h>

#include "websocket.hpp"
#include "request/request.hpp"
#include "response/response.hpp"
#include "template.hpp"
#include "log/log.hpp"
#include "ship.hpp"
#include "json.hpp"
#include "server/settings.hpp"
#include "server/server.hpp"
#include "trie.hpp"
#include "cookies/cookies.hpp"
#include "cookies/securecookies.hpp"
#include "middleware.hpp"

namespace harbour {

    namespace this_coro = asio::this_coro;
    using asio::awaitable;
    using asio::co_spawn;
    using asio::use_awaitable;

    class Harbour {
        /// @brief Display message of the day
        auto motd() {
            const std::string version = "Harbour: " + harbour_version;
            fmt::print(fmt::emphasis::bold | fg(fmt::color::aquamarine),
                       "┌{0:─^{3}}┐\n"
                       "│{2: ^{3}}│\n"
                       "│{1: ^{3}}│\n"
                       "└{0:∿^{3}}┘\n",
                       "", "Your ships are sailing", version, 24);

            fmt::print(fmt::emphasis::bold | fg(fmt::color::blue_violet), fmt::runtime("• Listening on: 0.0.0.0:{}\n"), settings.port);
            fmt::print(fmt::emphasis::bold | fg(fmt::color::bisque), "• Waiting for connections...\n");
        }

        /// @brief Apply ships to our Request and Response
        /// @param req Request to handle
        /// @param resp Response to handle
        auto handle_ships(Request &req, Response &resp) -> awaitable<void> {
            // Handle routed ships
            if (auto found = routes.match(req.path)) {
                auto &_ships      = found->node.value()->data;
                auto route_method = found->node.value()->method;
                req.route         = found->get_route();

                // Process routed ships if we dont have a Method constraint
                // If we do have a Method constraint, check if it matches the Requests Method
                if (!route_method || *route_method == req.method) {
                    for (auto &&ship: _ships) {
                        if (auto v = co_await detail::ShipHandler(req, resp, ship)) {
                            resp = *v;
                            break;
                        }
                    }
                }
            }

            // Handle global ships
            for (auto &&ship: ships) {
                if (auto v = co_await detail::ShipHandler(req, resp, ship)) {
                    resp = *v;
                    break;
                }
            }
        }

    public:
        /// @brief Construct Harbour using default settings
        [[nodiscard]] Harbour() : settings(server::Settings::defaults()) {}

        /// @brief Construct Harbour using settings
        /// @param settings Settings to configure Harbour with
        [[nodiscard]] Harbour(const server::Settings &settings) : settings(settings) {}

        /// @brief Dock Ship(s) as a global middleware
        /// @param ...ship Ship(s) to dock
        /// @return Chainable reference to Harbour
        constexpr auto dock(detail::ShipConcept auto... ship) -> Harbour & {
            (ships.emplace_back(detail::make_ship(ship)), ...);
            return *this;
        }

        /// @brief Dock Ship(s) to a given route
        /// @param route Route to dock our Ship(s)
        /// @param ...ship Ship(s) to dock
        /// @return Chainable reference to Harbour
        constexpr auto dock(const std::string &route, detail::ShipConcept auto... ship) -> Harbour & {
            std::vector<detail::Ship> routers;
            (routers.emplace_back(detail::make_ship(ship)), ...);
            routes.insert({}, route, std::move(routers));
            return *this;
        }

        /// @brief Dock Ship(s) to a given route with a Method constraint
        /// @param method Method constraint to use
        /// @param route Route to dock our Ship(s)
        /// @param ...ship Ship(s) to dock
        /// @return Chainable reference to Harbour
        constexpr auto dock(http::Method method, const std::string &route, detail::ShipConcept auto... ship) -> Harbour & {
            std::vector<detail::Ship> routers;
            (routers.emplace_back(detail::make_ship(ship)), ...);
            routes.insert(method, route, std::move(routers));
            return *this;
        }

        /// @brief Launch server and begin handling Ships
        auto sail() {
            motd();

            auto ship_handler = [&](Request &req, Response &resp) -> awaitable<void> { co_await handle_ships(req, resp); };
            server::Server srv(ship_handler, settings, ships);
            srv.serve();
        }

    private:
        server::Settings settings{server::Settings::defaults()};///< Settings for Server with defauls
        Trie<std::vector<detail::Ship>> routes;                 ///< Ships placed at routes
        std::vector<detail::Ship> ships;                        ///< Ships placed as global middleware
        const std::string harbour_version = "0.0.1";            ///< Server version
    };
}// namespace harbour
