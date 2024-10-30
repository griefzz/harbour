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
#include <string_view>

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
#include "middleware/middleware.hpp"

namespace harbour {

    namespace this_coro = asio::this_coro;
    using asio::awaitable;
    using asio::co_spawn;
    using asio::use_awaitable;

    class Harbour {
    public:
        /// @brief Construct Harbour using default settings
        [[nodiscard]] explicit Harbour() noexcept
            : settings_(server::Settings::defaults()) {}

        /// @brief Construct Harbour using settings
        /// @param settings Settings to configure Harbour with
        [[nodiscard]] explicit Harbour(const server::Settings &settings) noexcept
            : settings_{settings} {}

        /// @brief Dock Ship(s) as a global middleware
        /// @param ...ship Ship(s) to dock
        /// @return Chainable reference to Harbour
        template<detail::ShipConcept... Ships>
        constexpr auto &dock(Ships &&...ship) noexcept {
            (ships_.emplace_back(detail::make_ship(std::forward<Ships>(ship))), ...);
            return *this;
        }

        /// @brief Dock Ship(s) to a given route
        /// @param route Route to dock our Ship(s)
        /// @param ...ship Ship(s) to dock
        /// @return Chainable reference to Harbour
        template<detail::ShipConcept... Ships>
        constexpr auto &dock(std::string_view route, Ships &&...ship) {
            std::vector<detail::Ship> routers;
            routers.reserve(sizeof...(Ships));
            (routers.emplace_back(detail::make_ship(std::forward<Ships>(ship))), ...);
            routes_.insert({}, std::string{route}, std::move(routers));
            return *this;
        }

        /// @brief Dock Ship(s) to a given route with a Method constraint
        /// @param method Method constraint to use
        /// @param route Route to dock our Ship(s)
        /// @param ...ship Ship(s) to dock
        /// @return Chainable reference to Harbour
        template<detail::ShipConcept... Ships>
        constexpr auto &dock(http::MethodConstraint method, std::string_view route, Ships &&...ship) {
            std::vector<detail::Ship> routers;
            routers.reserve(sizeof...(Ships));
            (routers.emplace_back(detail::make_ship(std::forward<Ships>(ship))), ...);
            routes_.insert(method, std::string{route}, std::move(routers));
            return *this;
        }

        /// @brief Dock Ship(s) to a given route with a Method constraint
        /// @param method Method constraint to use
        /// @param route Route to dock our Ship(s)
        /// @param ...ship Ship(s) to dock
        /// @return Chainable reference to Harbour
        template<detail::ShipConcept... Ships>
        constexpr auto &dock(http::Method method, std::string_view route, Ships &&...ship) {
            return dock(static_cast<http::MethodConstraint>(method), route,
                        std::forward<Ships>(ship)...);
        }

        /// @brief Launch server and begin handling Ships
        void sail() {
            display_motd();

            auto ship_handler = [this](Request &req, Response &resp) -> awaitable<void> {
                co_await handle_ships(req, resp);
            };

            server::Server srv{ship_handler, settings_, ships_};
            srv.serve();
        }

    private:
        /// @brief Display message of the day
        void display_motd() const {
            static constexpr auto version = std::string_view{"Harbour: 0.0.1"};
            static constexpr auto width   = 24;

            fmt::print(fmt::emphasis::bold | fg(fmt::color::aquamarine),
                       "┌{0:─^{3}}┐\n"
                       "│{2: ^{3}}│\n"
                       "│{1: ^{3}}│\n"
                       "└{0:∿^{3}}┘\n",
                       "", "Your ships are sailing", version, width);

            fmt::print(fmt::emphasis::bold | fg(fmt::color::blue_violet),
                       fmt::runtime("• Listening on: 0.0.0.0:{}\n"), settings_.port);
            fmt::print(fmt::emphasis::bold | fg(fmt::color::bisque),
                       "• Waiting for connections...\n");
        }

        /// @brief Apply ships to our Request and Response
        /// @param req Request to handle
        /// @param resp Response to handle
        auto handle_ships(Request &req, Response &resp) -> awaitable<void> {
            // Handle routed ships first
            if (auto found = routes_.match(req.path)) {
                auto &ships     = found->node.value()->data;
                auto constraint = found->node.value()->method;
                req.route       = found->get_route();

                // Process routed ships if we dont have a Method constraint
                // or if the constraint matches the Request's Method
                if (!constraint || http::detail::matches_constraint(*constraint, req.method)) {
                    if (co_await try_handle_ships(ships, req, resp)) {
                        co_return;
                    }
                }
            }

            // Handle global ships if no route handled the request
            co_await try_handle_ships(ships_, req, resp);
        }

        /// @brief Try to handle ships until one returns a response
        /// @return true if a ship handled the request
        auto try_handle_ships(const std::vector<detail::Ship> &ships,
                              Request &req, Response &resp) -> awaitable<bool> {
            for (const auto &ship: ships) {
                if (auto response = co_await detail::ShipHandler(req, resp, ship)) {
                    resp = *response;
                    co_return true;
                }
            }
            co_return false;
        }

        server::Settings settings_{server::Settings::defaults()};
        Trie<std::vector<detail::Ship>> routes_;
        std::vector<detail::Ship> ships_;
    };

}// namespace harbour
