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
#include <string_view>
#include <future>
#include <array>
#include <vector>

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

namespace harbour::websocket {

    using asio::awaitable;
    using asio::co_spawn;
    using asio::use_awaitable;

    constexpr std::string_view WEBSOCKET_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    constexpr std::size_t DEFAULT_BUFFER_SIZE = 8192;

    /// @brief WebSocket frame opcodes
    enum class Opcode : uint8_t {
        Continuation = 0x0,
        Text         = 0x1,
        Binary       = 0x2,
        Close        = 0x8,
        Ping         = 0x9,
        Pong         = 0xA
    };

    /// @brief WebSocket Connection
    class Connection {
    public:
        Connection(std::string key, std::string version, std::string secret, server::SharedSocket socket)
            : key_(std::move(key)),
              version_(std::move(version)),
              secret_(std::move(secret)),
              socket_(std::move(socket)),
              read_buffer_(DEFAULT_BUFFER_SIZE) {}

        /// @brief Read a WebSocket frame from the connection
        /// @return String containing frame payload
        auto read() -> awaitable<std::optional<std::string>> {
            try {
                // Read header
                std::array<uint8_t, 2> header;
                auto bytes_read = co_await socket_->async_read_some(asio::buffer(header), use_awaitable);
                if (bytes_read != 2) co_return std::nullopt;

                bool fin             = (header[0] & 0x80) != 0;
                Opcode opcode        = static_cast<Opcode>(header[0] & 0x0F);
                bool masked          = (header[1] & 0x80) != 0;
                uint64_t payload_len = header[1] & 0x7F;

                // Handle extended payload length
                if (payload_len == 126) {
                    std::array<uint8_t, 2> ext_len;
                    bytes_read = co_await socket_->async_read_some(asio::buffer(ext_len), use_awaitable);
                    if (bytes_read != 2) co_return std::nullopt;
                    payload_len = (ext_len[0] << 8) | ext_len[1];
                } else if (payload_len == 127) {
                    std::array<uint8_t, 8> ext_len;
                    bytes_read = co_await socket_->async_read_some(asio::buffer(ext_len), use_awaitable);
                    if (bytes_read != 8) co_return std::nullopt;
                    payload_len = 0;
                    for (int i = 0; i < 8; i++) {
                        payload_len = (payload_len << 8) | ext_len[i];
                    }
                }

                // Read masking key if present
                std::array<uint8_t, 4> mask_key;
                if (masked) {
                    bytes_read = co_await socket_->async_read_some(asio::buffer(mask_key), use_awaitable);
                    if (bytes_read != 4) co_return std::nullopt;
                }

                // Read payload
                if (payload_len > read_buffer_.size()) {
                    read_buffer_.resize(payload_len);
                }
                bytes_read = co_await socket_->async_read_some(asio::buffer(read_buffer_.data(), payload_len), use_awaitable);
                if (bytes_read != payload_len) co_return std::nullopt;

                // Unmask if needed
                if (masked) {
                    for (size_t i = 0; i < payload_len; i++) {
                        read_buffer_[i] ^= mask_key[i % 4];
                    }
                }

                // Handle control frames
                switch (opcode) {
                    case Opcode::Close:
                        co_await close();
                        co_return std::nullopt;
                    case Opcode::Ping:
                        co_await send(read_buffer_.data(), payload_len, Opcode::Pong);
                        co_return std::string(read_buffer_.data(), payload_len);
                    default:
                        co_return std::string(read_buffer_.data(), payload_len);
                }

            } catch (const std::exception &) {
                co_return std::nullopt;
            }
        }

        /// @brief Send data over the WebSocket connection
        /// @param data Data to send
        /// @param len Length of data
        /// @param opcode Frame opcode
        /// @return Number of bytes sent
        // Base send method for raw pointer + length
        auto send(const void *data, size_t len, Opcode opcode = Opcode::Text) -> awaitable<std::size_t> {
            std::vector<uint8_t> frame;
            frame.reserve(len + 10);// Max header size + payload

            // Add header
            frame.push_back(0x80 | static_cast<uint8_t>(opcode));

            // Add payload length
            if (len <= 125) {
                frame.push_back(static_cast<uint8_t>(len));
            } else if (len <= 65535) {
                frame.push_back(126);
                frame.push_back((len >> 8) & 0xFF);
                frame.push_back(len & 0xFF);
            } else {
                frame.push_back(127);
                for (int i = 7; i >= 0; i--) {
                    frame.push_back((len >> (i * 8)) & 0xFF);
                }
            }

            // Add payload
            frame.insert(frame.end(), static_cast<const uint8_t *>(data),
                         static_cast<const uint8_t *>(data) + len);

            co_return co_await socket_->async_write(asio::buffer(frame), use_awaitable);
        }

        // Overload for string-like types
        template<typename T>
            requires std::ranges::contiguous_range<T> && std::ranges::sized_range<T>
        auto send(const T &data, Opcode opcode = Opcode::Text) -> awaitable<std::size_t> {
            return send(std::ranges::data(data), std::ranges::size(data), opcode);
        }

        /// @brief Close the WebSocket connection
        auto close() -> awaitable<void> {
            try {
                co_await send(nullptr, 0, Opcode::Close);
            } catch (...) {
                // Ignore errors during close
            }
        }

    private:
        std::string key_;
        std::string version_;
        std::string secret_;
        server::SharedSocket socket_;
        std::vector<char> read_buffer_;
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

        if (method != http::Method::GET || !socket || !upgrade || !key || !version) {
            co_return std::nullopt;
        }

        if (upgrade->find("Upgrade") == std::string::npos) {
            co_return std::nullopt;
        }

        // Generate accept key
        const auto handshake = fmt::format("{}{}", *key, WEBSOCKET_GUID);
        auto hash            = crypto::sha::sha1(handshake);
        auto secret          = crypto::base64::encode(hash);
        if (!secret) co_return std::nullopt;

        // Send upgrade response
        auto response = Response()
                                .with_status(http::Status::SwitchingProtocols)
                                .with_header("Upgrade", "websocket")
                                .with_header("Connection", "Upgrade")
                                .with_header("Sec-WebSocket-Accept", *secret);

        try {
            auto response_str  = response.string();
            auto bytes_written = co_await socket->async_write(asio::buffer(response_str), use_awaitable);
            if (bytes_written != response_str.size()) {
                co_return std::nullopt;
            }
        } catch (const std::exception &) {
            co_return std::nullopt;
        }

        co_return Connection{std::string(*key), std::string(*version), *secret, socket};
    }

}// namespace harbour::websocket
