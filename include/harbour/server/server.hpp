///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file server.hpp
/// @brief Contains the definition of the server and socket structures for handling connections.

#pragma once

#include <cstdio>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <array>

#include <asio.hpp>
#include <asio/ssl/impl/src.hpp>
#include <asio/ssl.hpp>

#include "socket.hpp"
#include "../response/response.hpp"
#include "../request/request.hpp"
#include "../ship.hpp"
#include "../log/log.hpp"

#if defined(ASIO_ENABLE_HANDLER_TRACKING)
    #define use_awaitable \
        asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

namespace harbour {
    namespace server {

        using asio::awaitable;
        using asio::co_spawn;
        using asio::detached;
        using asio::use_awaitable;
        using asio::ip::port_type;
        using asio::ip::tcp;
        namespace this_coro = asio::this_coro;
        namespace ssl       = asio::ssl;

        using ShipsHandleFn = std::function<awaitable<void>(Request &, Response &)>;

        /// @brief A structure representing the server.
        struct Server {
            /// @brief Constructs a Server object.
            /// @param fn The function to handle ship requests.
            /// @param settings The server Settings to use.
            /// @param ships The list of ships.
            [[nodiscard]] explicit Server(ShipsHandleFn fn, const Settings &settings, auto &ships)
                : settings(settings), handle_ships(std::move(fn)), ships(ships), ssl_context(nullptr) {
                if (settings.max_size < settings.buffering_size) {
                    log::critical("max_size size must be >= buffering_size");
                    exit(1);
                }

                const auto has_ssl_paths = settings.private_key_path && settings.certificate_path;
                const auto has_ssl_data  = settings.private_key && settings.certificate;

                try {
                    if (has_ssl_paths || has_ssl_data) {
                        ssl_context = std::make_unique<ssl::context>(ssl::context::sslv23);
                        if (!ssl_context) {
                            log::critical("Failed to initialize ssl::context");
                            exit(1);
                        }

                        ssl_context->set_verify_mode(ssl::verify_peer);
                        ssl_context->set_verify_mode(ssl::verify_client_once);

                        if (has_ssl_paths) {
                            ssl_context->use_certificate_chain_file(*settings.certificate_path);
                            ssl_context->use_private_key_file(*settings.private_key_path, ssl::context::pem);
                        } else if (has_ssl_data) {
                            ssl_context->use_certificate_chain(asio::buffer(*settings.certificate));
                            ssl_context->use_private_key(asio::buffer(*settings.private_key), ssl::context::pem);
                        }

                        ssl_context->set_options(ssl::context::default_workarounds | ssl::context::no_sslv2 | ssl::context::no_sslv3);
                    }
                } catch (const std::exception &e) {
                    log::critical("Failed to initialize Server: {}", e.what());
                    exit(1);
                }
            }

            /// @brief Handles a new connection.
            /// @param ctx The socket context.
            /// @return An awaitable object.
            auto on_connection(SharedSocket ctx) -> awaitable<void> {
                const auto addr                  = ctx->address();         // Client address
                const auto addr_port             = ctx->port();            // Client port
                const std::size_t max_size       = settings.max_size;      // Max size for an HTTP Request
                const std::size_t buffering_size = settings.buffering_size;// Size of temporary buffering buffer

                std::optional<std::string> warning_message; // Optional string holding the warning events message
                std::optional<std::string> critical_message;// Optional string holding the critical events message

                try {
                    if (settings.on_connection) co_await settings.on_connection(ctx);

                    std::string data;// Recieved HTTP Request data
                    data.reserve(buffering_size);

                    // Dynamically read up to max_size bytes from the socket stored into data
                    auto buffer = asio::dynamic_string_buffer(data, max_size);
                    co_await ctx->async_read(buffer, use_awaitable);

                    // Create a Request from recieved HTTP Request data
                    // Execute all ship handlers
                    // Return a Response to the client
                    if (auto req = Request::create(ctx, data.c_str(), data.size())) {
                        Response resp;
                        co_await handle_ships(*req, resp);
                        co_await ctx->async_write(resp.string(), use_awaitable);
                    } else {
                        // Parsing the request failed
                        // Send the client a 500 Internal Server Error as a Response
                        if (settings.on_warning) co_await settings.on_warning(ctx, fmt::format("Failed to parse request:\n{}", data));
                        co_await ctx->async_write(Response(http::Status::BadRequest).string(), use_awaitable);
                    }
                } catch (const asio::system_error &se) {
                    if (se.code() == asio::error::eof) {
                        // This is the normal result of trying to access a closed connection
                        // A ton of these warnings could indicate a larger problem
                        warning_message = "Connection closed early";
                    } else {
                        critical_message = fmt::format("asio exception: {}", se.what());
                    }
                } catch (const std::exception &e) {
                    critical_message = fmt::format("handle_ships exception: {}", e.what());
                }

                // Handle unexpected server event callback coroutines
                if (warning_message && settings.on_warning)
                    co_await settings.on_warning(ctx, *warning_message);

                if (critical_message && settings.on_critical)
                    co_await settings.on_critical(ctx, *critical_message);
            }

            /// @brief Listens for incoming connections.
            /// @return An awaitable object.
            auto listener() -> awaitable<void> {
                auto executor = co_await this_coro::executor;
                tcp::acceptor acceptor(executor, {tcp::v4(), settings.port});
                for (;;) {
                    try {
                        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
                        if (ssl_context) {
                            auto ctx = std::make_shared<Socket>(std::move(ssl::stream<tcp::socket>(std::move(socket), *ssl_context)));
                            co_await std::get<SslSocket>(ctx->socket).async_handshake(ssl::stream_base::server, use_awaitable);
                            co_spawn(executor, on_connection(ctx), detached);
                        } else {
                            auto ctx = std::make_shared<Socket>(std::move(socket));
                            co_spawn(executor, on_connection(ctx), detached);
                        }
                    } catch (const std::exception &e) { log::critical("Listener exception: {}", e.what()); }
                }
            }

            /// @brief Starts the server.
            auto serve() {
                try {
                    asio::io_context io_context(1);
                    asio::signal_set signals(io_context, SIGINT, SIGTERM);
                    signals.async_wait([&](auto, auto) { io_context.stop(); });

                    co_spawn(io_context, listener(), detached);

                    io_context.run();
                } catch (const std::exception &e) {
                    log::critical("Server exception: {}", e.what());
                }
            }

            Settings settings;                        ///< Settings for the Server
            ShipsHandleFn handle_ships;               ///< Function to handle Ships.
            std::vector<detail::Ship> &ships;         ///< Vector of global Ships.
            std::unique_ptr<ssl::context> ssl_context;///< SSL context for secure connections.
        };

    }// namespace server
}// namespace harbour
