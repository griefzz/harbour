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

namespace harbour {
    namespace server {

        /// @brief Type of the port used by Server
        using port_type = std::uint_least16_t;

        /// @brief Settings for Harbour's Server structure
        struct Settings {
            port_type port{8080};///< Port for server

            std::size_t max_size{8192};      ///< Maximum HTTP Request size
            std::size_t buffering_size{4096};///< Buffering size for HTTP Request (must be less than max_size)

            std::optional<std::string_view> private_key;///< Optional private key data
            std::optional<std::string_view> certificate;///< Optional certificate data

            std::optional<std::string> private_key_path;///< Optional private key path
            std::optional<std::string> certificate_path;///< Optional certificate path

            /// @brief Create a Settings with the default values
            /// @return Default Settings structure
            [[nodiscard]] static auto defaults() noexcept -> Settings {
                Settings s;
                s.port           = 8080;
                s.max_size       = 8192;
                s.buffering_size = 4096;
                return s;
            }

            /// @brief Set the server to use a specific port
            /// @param port Port to use
            /// @return Settings& Reference to Settings for chaining
            auto with_port(port_type port) noexcept -> Settings & {
                this->port = port;
                return *this;
            }

            /// @brief Set the maximum size for an HTTP Request
            /// @param max_size Size in bytes to use
            /// @return Settings& Reference to Settings for chaining
            auto with_max_size(std::size_t max_size) noexcept -> Settings & {
                this->max_size = max_size;
                return *this;
            }

            /// @brief Set the size of the HTTP Request temporary buffer (must be >= max_size)
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
        };

    }// namespace server
}// namespace harbour