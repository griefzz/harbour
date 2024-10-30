///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file socket.hpp
/// @brief Contains the implementation details for the harbour Socket type

#pragma once

#include <string>
#include <type_traits>
#include <variant>
#include <concepts>

#include <asio.hpp>
#include <asio/ssl/impl/src.hpp>
#include <asio/ssl.hpp>

namespace harbour::server {

    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;
    using asio::use_awaitable;
    using asio::ip::port_type;
    using asio::ip::tcp;
    namespace ssl = asio::ssl;

    using TcpSocket = tcp::socket;
    using SslSocket = ssl::stream<TcpSocket>;

    /// @brief A structure representing a socket, which can be either a plain TCP socket or an SSL socket.
    class Socket final : public std::enable_shared_from_this<Socket> {
    public:
        // Constructor for TCP socket
        explicit Socket(tcp::socket &&socket) noexcept : socket_(std::move(socket)) {}

        // Constructor for SSL stream
        explicit Socket(ssl::stream<TcpSocket> &&socket) noexcept : socket_(std::move(socket)) {}

        // Delete copy operations
        Socket(const Socket &)            = delete;
        Socket &operator=(const Socket &) = delete;

        // Allow move operations
        Socket(Socket &&) noexcept            = default;
        Socket &operator=(Socket &&) noexcept = default;

        /// @brief Gets the remote address of the socket.
        /// @return The remote address as a string.
        [[nodiscard]] auto address() const -> std::string {
            return std::visit([](const auto &sock) {
                if constexpr (std::is_same_v<std::decay_t<decltype(sock)>, TcpSocket>) {
                    return sock.remote_endpoint().address().to_string();
                } else {
                    return sock.lowest_layer().remote_endpoint().address().to_string();
                }
            },
                              socket_);
        }

        /// @brief Gets the remote port of the socket.
        /// @return The remote port.
        [[nodiscard]] auto port() const -> port_type {
            return std::visit([](const auto &sock) {
                if constexpr (std::is_same_v<std::decay_t<decltype(sock)>, TcpSocket>) {
                    return sock.remote_endpoint().port();
                } else {
                    return sock.lowest_layer().remote_endpoint().port();
                }
            },
                              socket_);
        }

        /// @brief Get the number of bytes available to read on a socket
        /// @return Number of available bytes
        [[nodiscard]] auto available() const -> std::size_t {
            return std::visit([](const auto &sock) {
                if constexpr (std::is_same_v<std::decay_t<decltype(sock)>, TcpSocket>) {
                    return sock.available();
                } else {
                    return sock.lowest_layer().available();
                }
            },
                              socket_);
        }

        /// @brief Get a reference to the underlying socket variant
        /// @return Reference to the socket variant containing either a TCP or SSL socket
        auto socket() -> std::variant<TcpSocket, SslSocket> & { return socket_; }

        /// @brief Asynchronously reads some data from the socket.
        /// @param buffers The buffer(s) into which the data will be read.
        /// @param token The completion token to be called when the operation completes.
        /// @return The result of the asynchronous read operation.
        template<typename BuffersType, typename CompletionToken>
        auto async_read_some(BuffersType &&buffers, CompletionToken &&token) {
            return std::visit([&](auto &sock) {
                return sock.async_read_some(std::forward<BuffersType>(buffers),
                                            std::forward<CompletionToken>(token));
            },
                              socket_);
        }

        /// @brief Reads some data from the socket.
        /// @param buffers The buffer(s) into which the data will be read.
        /// @return The number of bytes read.
        template<typename BuffersType>
        auto read_some(BuffersType &&buffers) {
            return std::visit([&](auto &sock) {
                return sock.read_some(std::forward<BuffersType>(buffers));
            },
                              socket_);
        }

        /// @brief Wrapper for asio::async_read(..., ..., asio::transfer_at_least(1), ...)
        /// @tparam BuffersType Type of buffer to use (asio::buffer, asio::dynamic_string_buffer)
        /// @tparam CompletionToken Completion token to use (asio::use_awaitable, asio::use_future)
        /// @param buffers Buffer to use
        /// @param token Completion token to use
        /// @return Number of bytes read
        template<typename BuffersType, asio::completion_token_for<void(std::error_code, std::size_t)> CompletionToken>
        auto async_read(BuffersType &&buffers, CompletionToken &&token) {
            return std::visit([&](auto &sock) {
                return asio::async_read(sock, std::forward<BuffersType>(buffers),
                                        asio::transfer_at_least(1),
                                        std::forward<CompletionToken>(token));
            },
                              socket_);
        }

        /// @brief Asynchronously writes a message to the socket.
        /// @tparam T The type of the message to be written.
        /// @tparam CompletionToken Completion token to use (asio::use_awaitable, asio::use_future)
        /// @param message The message to be written.
        /// @param token The completion token to be called when the operation completes.
        /// @return The result of the asynchronous write operation.
        template<typename T, asio::completion_token_for<void(std::error_code, std::size_t)> CompletionToken>
        auto async_write(T &&message, CompletionToken &&token) {
            return std::visit([&](auto &sock) {
                return asio::async_write(sock,
                                         asio::buffer(std::forward<T>(message)),
                                         std::forward<CompletionToken>(token));
            },
                              socket_);
        }

        /// @brief Writes some data to the socket.
        /// @param buffers The buffer(s) containing the data to be written.
        /// @return The number of bytes written.
        template<typename BuffersType>
        auto write_some(BuffersType &&buffers) {
            return std::visit([&](auto &sock) {
                return sock.write_some(std::forward<BuffersType>(buffers));
            },
                              socket_);
        }

        /// @brief Gets a shared pointer to the socket.
        /// @return A shared pointer to the socket.
        [[nodiscard]] auto getptr() -> std::shared_ptr<Socket> {
            return shared_from_this();
        }

    private:
        std::variant<TcpSocket, SslSocket> socket_;
    };

    /// @brief Convenience type for a std::shared_ptr<Socket>
    using SharedSocket = std::shared_ptr<Socket>;

}// namespace harbour::server