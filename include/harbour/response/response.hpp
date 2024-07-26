///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file response.hpp
/// @brief Contains the implementation of harbours HTTP Response type

#pragma once

#include <string>
#include <optional>

#include <fmt/core.h>
#include <fmt/format.h>

#include "headers.hpp"
#include "../http/status.hpp"
#include "../json.hpp"
#include "../cookies/cookies.hpp"

namespace harbour {

    /// @brief Structure representing an HTTP response.
    struct Response {
        http::Status status{http::Status::OK};///< HTTP status code
        response::Headers headers;            ///< HTTP headers.
        Cookies cookies;                      ///< Cookie data
        std::optional<std::string> data;      ///< Optional response data.

        /// @brief Default constructor.
        [[nodiscard]] Response() = default;

        /// @brief Constructor with status.
        /// @param status HTTP status code.
        [[nodiscard]] Response(http::Status status) : status(status) {}

        /// @brief Constructor with string data.
        /// @param data Response data as a string.
        [[nodiscard]] Response(const std::string &data) : status(http::Status::OK), data(data) { headers["Content-Type"] = "text/html; charset=utf-8"; }

        /// @brief Constructor with string_view data.
        /// @param data Response data as a string.
        [[nodiscard]] Response(std::string_view data) : status(http::Status::OK), data(data) { headers["Content-Type"] = "text/html; charset=utf-8"; }

        /// @brief Constructor with C-string data.
        /// @param data Response data as a C-string.
        [[nodiscard]] Response(const char *data) : status(http::Status::OK), data(data) { headers["Content-Type"] = "text/html; charset=utf-8"; }

        /// @brief Constructor with JSON data.
        /// @param js Response data as JSON.
        [[nodiscard]] Response(const json::json_t &js) : status(http::Status::OK), data(js.dump()) { headers["Content-Type"] = "application/json"; }

        /// @brief Set response data.
        /// @param data Response data as a string.
        /// @return Reference to the modified Response object.
        [[nodiscard]] auto with_data(const std::string &data) noexcept -> Response & {
            this->data = data;
            return *this;
        }

        /// @brief Set HTTP status.
        /// @param status HTTP status code.
        /// @return Reference to the modified Response object.
        [[nodiscard]] auto with_status(http::Status status) noexcept -> Response & {
            this->status = status;
            return *this;
        }

        /// @brief Set a single header.
        /// @param key Header key.
        /// @param value Header value.
        /// @return Reference to the modified Response object.
        [[nodiscard]] auto with_header(const std::string &key, const ::std::string value) noexcept -> Response & {
            headers[key] = value;
            return *this;
        }

        /// @brief Set multiple headers.
        /// @param headers List of headers as key-value pairs.
        /// @return Reference to the modified Response object.
        [[nodiscard]] auto with_headers(const std::initializer_list<std::pair<std::string, std::string>> &headers) noexcept -> Response & {
            for (const auto &h: headers) this->headers[h.first] = h.second;
            return *this;
        }

        /// @brief Set headers from a Headers object.
        /// @param headers Headers object.
        /// @return Reference to the modified Response object.
        [[nodiscard]] auto with_headers(const response::Headers &headers) noexcept -> Response & {
            this->headers = headers;
            return *this;
        }

        /// @brief Set a redirect location.
        /// @param location Redirect location URL.
        /// @return Reference to the modified Response object.
        [[nodiscard]] auto with_redirect(const std::string &location) noexcept -> Response & {
            status              = http::Status::Found;
            headers["Location"] = location;
            return *this;
        }

        /// @brief Set the cookies of a Response
        /// @param c Cookies to set
        /// @return Reference to the modified Response object.
        [[nodiscard]] auto with_cookies(const Cookies &c) noexcept -> Response & {
            cookies = c;
            return *this;
        }

        /// @brief Access header value by key.
        /// @param key Header key.
        /// @return Reference to the header value.
        constexpr auto operator[](const auto &key) -> std::string & {
            return headers[key];
        }

        /// @brief Convert the response to a string.
        /// @return Response as a string.
        [[nodiscard]] auto string() -> std::string {
            std::string resp;

            // Status
            resp += fmt::format("{} {}\n", "HTTP/1.1", status);

            // Headers
            if (!headers.empty())
                resp += fmt::format("{}", headers);

            // Cookies
            if (!cookies.data.empty())
                resp += fmt::format("Set-Cookie: {}\n", cookies);

            // Connection
            resp += fmt::format("Connection: keep-alive\n");

            // Data
            if (data)
                resp += fmt::format("Content-Length: {}\n\n{}", data->size(), *data);
            else
                resp += "\n";

            return resp;
        }
    };

}// namespace harbour