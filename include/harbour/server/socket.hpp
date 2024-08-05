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

#include <asio.hpp>
#include <asio/ssl/impl/src.hpp>
#include <asio/ssl.hpp>

namespace harbour {
    namespace server {

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
        struct Socket : std::enable_shared_from_this<Socket> {
            std::variant<TcpSocket, SslSocket> socket;

            // Constructor for TCP socket
            constexpr explicit Socket(tcp::socket &&socket) : socket(std::move(socket)) {}

            // Constructor for SSL stream
            constexpr explicit Socket(ssl::stream<TcpSocket> &&socket) : socket(std::move(socket)) {}

            /// @brief Gets the remote address of the socket.
            /// @return The remote address as a string.
            auto address() const -> std::string {
                if (std::holds_alternative<TcpSocket>(socket))
                    return std::get<TcpSocket>(socket).remote_endpoint().address().to_string();
                else
                    return std::get<SslSocket>(socket).lowest_layer().remote_endpoint().address().to_string();
            }

            /// @brief Gets the remote address of the socket.
            /// @return The remote address as a string.
            auto port() const -> port_type {
                if (std::holds_alternative<TcpSocket>(socket))
                    return std::get<TcpSocket>(socket).remote_endpoint().port();
                else
                    return std::get<SslSocket>(socket).lowest_layer().remote_endpoint().port();
            }

            /// @brief Get the number of bytes available to read on a socket
            /// @return Number of available bytes
            auto available() const -> std::size_t {
                if (std::holds_alternative<TcpSocket>(socket))
                    return std::get<TcpSocket>(socket).available();
                else
                    return std::get<SslSocket>(socket).lowest_layer().available();
            }

            /// @brief Asynchronously reads some data from the socket.
            /// @param buffers The buffer(s) into which the data will be read.
            /// @param token The completion token to be called when the operation completes.
            /// @return The result of the asynchronous read operation.
            auto async_read_some(auto &&buffers, auto &&token) {
                if (std::holds_alternative<TcpSocket>(socket))
                    return std::get<TcpSocket>(socket).async_read_some(buffers, token);
                else
                    return std::get<SslSocket>(socket).async_read_some(buffers, token);
            }

            /// @brief Reads some data from the socket.
            /// @param buffers The buffer(s) into which the data will be read.
            /// @return The number of bytes read.
            auto read_some(auto &&buffers) {
                if (std::holds_alternative<TcpSocket>(socket))
                    return std::get<TcpSocket>(socket).read_some(buffers);
                else
                    return std::get<SslSocket>(socket).read_some(buffers);
            }

            /// @brief Wrapper for asio::async_read(..., ..., asio::transfer_at_least(1), ...)
            /// @tparam BuffersType Type of buffer to use (asio::buffer, asio::dynamic_string_buffer)
            /// @tparam CompletionToken Completion token to use (asio::use_awaitable, asio::use_future)
            /// @param buffers Buffer to use
            /// @param token Completion token to use
            /// @return Number of bytes read
            template<typename BuffersType, asio::completion_token_for<void(std::error_code, std::size_t)> CompletionToken>
            auto async_read(BuffersType buffers, CompletionToken &&token) {
                if (std::holds_alternative<TcpSocket>(socket)) {
                    auto &s = std::get<TcpSocket>(socket);
                    return asio::async_read(s, buffers, asio::transfer_at_least(1), token);
                } else {
                    auto &s = std::get<SslSocket>(socket);
                    return asio::async_read(s, buffers, asio::transfer_at_least(1), token);
                }
            }

            /// @brief Asynchronously writes a message to the socket.
            /// @tparam T The type of the message to be written.
            /// @tparam CompletionToken Completion token to use (asio::use_awaitable, asio::use_future)
            /// @param message The message to be written.
            /// @param token The completion token to be called when the operation completes.
            /// @return The result of the asynchronous write operation.
            template<typename T, asio::completion_token_for<void(std::error_code, std::size_t)> CompletionToken>
            auto async_write(T &&message, CompletionToken &&token) {
                if (std::holds_alternative<TcpSocket>(socket))
                    return asio::async_write(std::get<TcpSocket>(socket),
                                             asio::buffer(std::forward<T>(message)),
                                             std::forward<CompletionToken>(token));
                else
                    return asio::async_write(std::get<SslSocket>(socket),
                                             asio::buffer(std::forward<T>(message)),
                                             std::forward<CompletionToken>(token));
            }

            /// @brief Writes some data to the socket.
            /// @param buffers The buffer(s) containing the data to be written.
            /// @return The number of bytes written.
            auto write_some(auto &&buffers) {
                if (std::holds_alternative<TcpSocket>(socket))
                    return std::get<TcpSocket>(socket).write_some(buffers);
                else
                    return std::get<SslSocket>(socket).write_some(buffers);
            }

            /// @brief Gets a shared pointer to the socket.
            /// @return A shared pointer to the socket.
            auto getptr() -> std::shared_ptr<Socket> {
                return shared_from_this();
            }
        };

        /// @brief Convenience type for a std::shared_ptr<Socket>
        using SharedSocket = std::shared_ptr<Socket>;

    }// namespace server
}// namespace harbour