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

namespace harbour::server {

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
            : settings_(std::move(settings)), handle_ships_(std::move(fn)), ships_(ships), ssl_context_(nullptr) {

            validate_settings();
            initialize_ssl_context();
        }

        void validate_settings() const {
            if (settings_.max_size < settings_.buffering_size) {
                log::critical("max_size size must be >= buffering_size");
                exit(1);
            }
        }

        void initialize_ssl_context() {
            const auto has_ssl_paths = settings_.private_key_path && settings_.certificate_path;
            const auto has_ssl_data  = settings_.private_key && settings_.certificate;

            if (!has_ssl_paths && !has_ssl_data) {
                return;
            }

            try {
                create_ssl_context();
                configure_ssl_context();
                load_certificates(has_ssl_paths);
            } catch (const std::exception &e) {
                log::critical("Failed to initialize Server: {}", e.what());
                exit(1);
            }
        }

        void create_ssl_context() {
            ssl_context_ = std::make_unique<ssl::context>(ssl::context::sslv23);
            if (!ssl_context_) {
                log::critical("Failed to initialize ssl::context");
                exit(1);
            }
        }

        void configure_ssl_context() {
            ssl_context_->set_verify_mode(ssl::verify_peer);
            ssl_context_->set_verify_mode(ssl::verify_client_once);

            if (settings_.private_key_password) {
                auto cb = [password = *settings_.private_key_password](std::size_t, ssl::context::password_purpose) -> std::string {
                    return password;
                };

                asio::error_code ec;
                ssl_context_->set_password_callback(cb, ec);
                if (ec) {
                    log::critical("Failed to set password callback: {}", ec.message());
                    exit(1);
                }
            }

            ssl_context_->set_options(
                    ssl::context::default_workarounds |
                    ssl::context::no_sslv2 |
                    ssl::context::no_sslv3);
        }

        void load_certificates(bool using_paths) {
            if (using_paths) {
                ssl_context_->use_certificate_chain_file(*settings_.certificate_path);
                ssl_context_->use_private_key_file(*settings_.private_key_path, ssl::context::pem);
            } else {
                ssl_context_->use_certificate_chain(asio::buffer(*settings_.certificate));
                ssl_context_->use_private_key(asio::buffer(*settings_.private_key), ssl::context::pem);
            }
        }

        /// @brief Handles a new connection.
        /// @param ctx The socket context.
        /// @return An awaitable object.
        auto on_connection(SharedSocket ctx) -> awaitable<void> {
            std::optional<std::exception> handle_ships_exception;
            std::optional<asio::system_error> asio_exception;

            try {
                if (settings_.on_connection) {
                    co_await settings_.on_connection(ctx);
                }

                std::string data;
                data.reserve(settings_.buffering_size);

                auto buffer = asio::dynamic_string_buffer(data, settings_.max_size);
                co_await ctx->async_read(buffer, use_awaitable);

                if (auto request = Request::create(ctx, data.c_str(), data.size())) {
                    Response response;
                    co_await handle_ships_(*request, response);
                    co_await ctx->async_write(response.string(), use_awaitable);
                } else {
                    co_await handle_failed_request(ctx, data);
                }
            } catch (const asio::system_error &se) {
                asio_exception = se;
            } catch (const std::exception &e) {
                handle_ships_exception = e;
            }

            if (asio_exception) {
                co_await handle_connection_error(ctx, *asio_exception);
            } else if (handle_ships_exception) {
                co_await handle_critical_error(ctx, *handle_ships_exception);
            }
        }

        // New helper methods to break down the connection handling
        auto handle_failed_request(const SharedSocket &ctx, const std::string &data) -> awaitable<void> {
            if (settings_.on_warning) {
                co_await settings_.on_warning(ctx, fmt::format("Failed to parse request:\n{}", data));
            }
            co_await ctx->async_write(Response(http::Status::BadRequest).string(), use_awaitable);
        }

        auto handle_connection_error(const SharedSocket &ctx, const asio::system_error &se) -> awaitable<void> {
            if (se.code() == asio::error::eof) {
                if (settings_.on_warning) {
                    co_await settings_.on_warning(ctx, "Connection closed early");
                }
            } else if (settings_.on_critical) {
                co_await settings_.on_critical(ctx, fmt::format("asio exception: {}", se.what()));
            }
        }

        auto handle_critical_error(const SharedSocket &ctx, const std::exception &e) -> awaitable<void> {
            if (settings_.on_critical) {
                co_await settings_.on_critical(ctx, fmt::format("handle_ships exception: {}", e.what()));
            }
        }

        auto listener() -> awaitable<void> {
            auto executor = co_await this_coro::executor;
            tcp::acceptor acceptor(executor, {tcp::v4(), settings_.port});

            while (true) {
                try {
                    tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
                    handle_new_connection(std::move(socket), executor);
                } catch (const std::exception &e) {
                    log::critical("Listener exception: {}", e.what());
                }
            }
        }

        template<typename Executor>
        void handle_new_connection(tcp::socket &&socket, const Executor &executor) {
            if (ssl_context_) {
                auto ctx = std::make_shared<Socket>(std::move(ssl::stream<tcp::socket>(std::move(socket), *ssl_context_)));
                co_spawn(executor, handle_ssl_connection(ctx), detached);
            } else {
                auto ctx = std::make_shared<Socket>(std::move(socket));
                co_spawn(executor, on_connection(ctx), detached);
            }
        }

        auto handle_ssl_connection(SharedSocket ctx) -> awaitable<void> {
            co_await std::get<SslSocket>(ctx->socket()).async_handshake(ssl::stream_base::server, use_awaitable);
            co_await on_connection(ctx);
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

        Settings settings_;                        ///< Settings for the Server
        ShipsHandleFn handle_ships_;               ///< Function to handle Ships.
        std::vector<detail::Ship> &ships_;         ///< Vector of global Ships.
        std::unique_ptr<ssl::context> ssl_context_;///< SSL context for secure connections.
    };

}// namespace harbour::server
