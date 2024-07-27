///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file callbacks.hpp
/// @brief Contains the implementation of harbours logger callbacks

#pragma once

#include <functional>
#include <string_view>

#include "../request/request.hpp"
#include "../server/socket.hpp"

namespace harbour {
    namespace log {
        namespace callbacks {

            /// @brief Callback type for a new connection. Contains a Shared Socket for the connection
            using Connection = std::function<void(std::shared_ptr<server::Socket>)>;

            /// @brief Callback type for a server warning. Contains a Shared Socket
            ///        for the connection and a message describing the event
            using Warning = std::function<void(std::shared_ptr<server::Socket>, const std::string_view)>;

            /// @brief Callback type for a server critical. Contains a Shared Socket
            ///        for the connection and a message describing the event
            using Critical = std::function<void(std::shared_ptr<server::Socket>, const std::string_view)>;

            /// @brief Default callback for new connections. Will print ip:port -> Connected
            /// @param req Shared Socket to use for callback.
            static auto on_connection(std::shared_ptr<server::Socket> socket) -> void {
                log::info("{}:{} → Connected", socket->address(), socket->port());
            }

            /// @brief Default callback for server warnings. Will print ip:port -> [message]
            /// @param socket Shared Socket to use for callback.
            /// @param message Message to describe the event
            static auto on_warning(std::shared_ptr<server::Socket> socket, const std::string_view message) -> void {
                log::warn("{}:{} → {}", socket->address(), socket->port(), message);
            }

            /// @brief Default callback for server criticals. Will print ip:port -> [message]
            /// @param socket Shared Socket to use for callback
            /// @param message Message to describe the event
            static auto on_critical(std::shared_ptr<server::Socket> socket, const std::string_view message) {
                log::critical("{}:{} → {}", socket->address(), socket->port(), message);
            }

        }// namespace callbacks
    }// namespace log
}// namespace harbour