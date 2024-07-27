///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file websocket.hpp
/// @brief Contains the implementation details for Harbour's websockets

#pragma once

#include <optional>
#include <span>
#include <string>
#include <future>

#include <asio/compose.hpp>
#include <asio/coroutine.hpp>
#include <asio/deferred.hpp>
#include <asio/io_context.hpp>
#include <asio/use_future.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/signal_set.hpp>
#include <asio/write.hpp>
#include <asio/ssl/impl/src.hpp>
#include <asio/ssl.hpp>

#if defined(ASIO_ENABLE_HANDLER_TRACKING)
    #define use_awaitable \
        asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

#include <fmt/format.h>

#include "server/socket.hpp"
#include "request/request.hpp"
#include "response/response.hpp"
#include "crypto/base64.hpp"
#include "crypto/sha.hpp"

namespace harbour {
    namespace websocket {

        using asio::awaitable;
        using asio::co_spawn;
        using asio::use_awaitable;

        /// @brief WebSocket Connection
        struct Connection {
            std::string key;                       ///< Upgrade request key
            std::string version;                   ///< Websocket version
            std::string secret;                    ///< Shared server/client secret
            server::SharedSocket socket;///< Underlying socket connection

            /// @brief Read from a connection returning the result
            /// @param n Maximum number of bytes to read
            /// @return String response from connection
            auto read(std::size_t n) -> awaitable<std::string> {
                auto data = std::make_unique_for_overwrite<char[]>(n);
                auto got  = co_await socket->async_read_some(asio::buffer(data.get(), n), use_awaitable);
                co_return std::string(data.get(), got);
            }

            /// @brief Write to a connection returing bytes written
            /// @param data Data to send to client
            /// @return Number of bytes sent
            auto write(const std::string &data) -> awaitable<std::size_t> {
                co_return co_await socket->async_write(asio::buffer(data.c_str(), data.size()), use_awaitable);
            }
        };

        /// @brief Upgrade a client to a websocket connection
        /// @param req Request used to determine if client wants to upgrade
        /// @return An awaitable Connection (empty if failed to upgrade client)
        auto upgrade(const Request &req) -> awaitable<std::optional<Connection>> {
            auto socket  = req.socket;
            auto method  = req.method;
            auto upgrade = req.header("Connection");
            auto key     = req.header("Sec-WebSocket-Key");
            auto version = req.header("Sec-WebSocket-Version");

            if (method != http::Method::GET) co_return std::nullopt;
            if (!upgrade) co_return std::nullopt;
            if (upgrade->find("Upgrade") == std::string::npos) co_return std::nullopt;
            if (!key || !version || !socket) co_return std::nullopt;

            // Combine the clients key with the websocket magic string
            // then sha1 hash the result and send the base64 encoding of the
            // hash to the client within an HTTP Switching Protocols response
            const auto handshake = fmt::format("{}{}", *key, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
            auto hash            = crypto::sha::sha1(handshake);
            auto secret          = crypto::base64::encode(hash);
            if (!secret) co_return std::nullopt;

            auto protoswitch = Response()
                                       .with_status(http::Status::SwitchingProtocols)
                                       .with_header("Upgrade", "websocket")
                                       .with_header("Connection", "Upgrade")
                                       .with_header("Sec-WebSocket-Accept", *secret);

            Connection conn{std::string(*key), std::string(*version), *secret, socket};

            try {
                auto protoswitch_str = protoswitch.string();
                auto n               = co_await conn.write(protoswitch_str);
                if (n != protoswitch_str.size()) co_return std::nullopt;
            } catch (const std::exception &e) { co_return std::nullopt; }

            co_return conn;
        }

    }// namespace websocket
}// namespace harbour
