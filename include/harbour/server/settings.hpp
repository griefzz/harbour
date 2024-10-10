///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file settings.hpp
/// @brief Contains the definition of the server settings structure.

#pragma once

#include <string>
#include <vector>
#include <optional>

#include "../log/callbacks.hpp"

namespace harbour::server {

    /// @brief Type of the port used by Server
    using port_type = std::uint_least16_t;

    /// @brief Settings for Harbour's Server structure
    struct Settings {
        port_type port{8080};///< Port for server

        std::size_t max_size{8192};      ///< Maximum HTTP Request size. (must be greater than or equal to buffering_size)
        std::size_t buffering_size{4096};///< Default allocation size for HTTP Requests (must be less than or equal to max_size)

        std::optional<std::string_view> private_key;///< Optional private key data
        std::optional<std::string_view> certificate;///< Optional certificate data

        std::optional<std::string> private_key_path;///< Optional private key path
        std::optional<std::string> certificate_path;///< Optional certificate path

        log::callbacks::Connection on_connection{log::callbacks::on_connection};///< Callback for a new connection
        log::callbacks::Warning on_warning{log::callbacks::on_warning};         ///< Callback for a server warning
        log::callbacks::Critical on_critical{log::callbacks::on_critical};      ///< Callback for a server critical

        /// @brief Create a Settings with the default values
        /// @return Default Settings structure
        [[nodiscard]] static auto defaults() noexcept -> Settings {
            Settings s;
            s.port           = 8080;
            s.max_size       = 8192;
            s.buffering_size = 4096;
            s.on_connection  = log::callbacks::on_connection;
            s.on_warning     = log::callbacks::on_warning;
            s.on_critical    = log::callbacks::on_critical;
            return s;
        }

        /// @brief Set the server to use a specific port
        /// @param port Port to use
        /// @return Settings& Reference to Settings for chaining
        auto with_port(port_type port) noexcept -> Settings & {
            this->port = port;
            return *this;
        }

        /// @brief Set the maximum size for an HTTP Request (must be >= buffering_size)
        /// @param max_size Size in bytes to use
        /// @return Settings& Reference to Settings for chaining
        auto with_max_size(std::size_t max_size) noexcept -> Settings & {
            this->max_size = max_size;
            return *this;
        }

        /// @brief Set the initial size of the HTTP Request buffer (must be <= max_size).
        ///        Harbour will dynamically grow from this amount up to max_size.
        ///        You should set this value to your average HTTP Request size for best performance.
        /// @param buffering_size Size in bytes to use
        /// @return Settings& Reference to Settings for chaining
        auto with_buffering_size(std::size_t buffering_size) noexcept -> Settings & {
            this->buffering_size = buffering_size;
            return *this;
        }

        /// @brief Set the PEM format SSL certificate and private key using data stored in memory
        /// @param certificate Certificate to use
        /// @param private_key Private key to use
        /// @return Settings& Reference to Settings for chaining
        auto with_ssl_data(std::string_view certificate, std::string_view private_key) noexcept -> Settings & {
            this->certificate = certificate;
            this->private_key = private_key;
            return *this;
        }

        /// @brief Set the PEM format SSL certificate and private key using files
        /// @param certificate_path Path to certificate
        /// @param private_key_path Path to private key
        /// @return Settings& Reference to Settings for chaining
        auto with_ssl_paths(std::string_view certificate_path, std::string_view private_key_path) noexcept -> Settings & {
            this->certificate_path = certificate_path;
            this->private_key_path = private_key_path;
            return *this;
        }

        /// @brief Set the new connection event callback
        /// @param on_connection Callback to set. If nullptr, will not be set.
        /// @return Settings& Reference to Settings for chaining
        auto with_on_connection(log::callbacks::Connection on_connection) -> Settings & {
            this->on_connection = on_connection;
            return *this;
        }

        /// @brief Set the server warning event callback
        /// @param on_warning Callback to set. If nullptr, will not be set.
        /// @return Settings& Reference to Settings for chaining
        auto with_on_warning(log::callbacks::Warning on_warning) -> Settings & {
            this->on_warning = on_warning;
            return *this;
        }

        /// @brief Set the server critical event callback
        /// @param on_critical Callback to set. If nullptr, will not be set
        /// @return Settings& Reference to Settings for chaining
        auto with_on_critical(log::callbacks::Critical on_critical) -> Settings & {
            this->on_critical = on_critical;
            return *this;
        }
    };

}// namespace harbour::server