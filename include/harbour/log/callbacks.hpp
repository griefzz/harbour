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

namespace harbour::log::callbacks {

    using server::SharedSocket;

    /// @brief Callback coroutine type for a new connection. Contains a Shared Socket for the connection
    using Connection = std::function<asio::awaitable<void>(SharedSocket)>;

    /// @brief Callback coroutine type for a server warning. Contains a Shared Socket
    ///        for the connection and a message describing the event
    using Warning = std::function<asio::awaitable<void>(SharedSocket, const std::string_view)>;

    /// @brief Callback coroutine type for a server critical. Contains a Shared Socket
    ///        for the connection and a message describing the event
    using Critical = std::function<asio::awaitable<void>(SharedSocket, const std::string_view)>;

    /// @brief Default callback coroutine for new connections. Will print ip:port -> Connected
    /// @param req Shared Socket to use for callback.
    /// @return asio::awaitable<void> Convert function to a coroutine
    static auto on_connection(SharedSocket socket) -> asio::awaitable<void> {
        log::info("{}:{} → Connected", socket->address(), socket->port());
        co_return;
    }

    /// @brief Default callback coroutine for server warnings. Will print ip:port -> [message]
    /// @param socket Shared Socket to use for callback.
    /// @param message Message to describe the event
    /// @return asio::awaitable<void> Convert function to a coroutine
    static auto on_warning(SharedSocket socket, const std::string_view message) -> asio::awaitable<void> {
        log::warn("{}:{} → {}", socket->address(), socket->port(), message);
        co_return;
    }

    /// @brief Default callback coroutine for server criticals. Will print ip:port -> [message]
    /// @param socket Shared Socket to use for callback
    /// @param message Message to describe the event
    /// @return asio::awaitable<void> Convert function to a coroutine
    static auto on_critical(SharedSocket socket, const std::string_view message) -> asio::awaitable<void> {
        log::critical("{}:{} → {}", socket->address(), socket->port(), message);
        co_return;
    }

}// namespace harbour::log::callbacks