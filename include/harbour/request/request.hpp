///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file request.hpp
/// @brief Contains the implementation of harbours http request type

#pragma once

#include <string>
#include <optional>
#include <format>
#include <iostream>
#include <vector>
#include <span>

#include <llhttp.h>

#include "parser.hpp"
#include "forms.hpp"
#include "headers.hpp"
#include "../http/method.hpp"
#include "../server/socket.hpp"
#include "../log/log.hpp"

namespace harbour {

    /// @brief Represents an HTTP request.
    struct Request {
        /// @brief Optional route information type containing the Key and Value for a route as a pair
        using Route = std::optional<std::pair<std::string, std::string>>;

        /// @brief Creates a Request object from raw http request data.
        /// @param sock The underlying socket connection.
        /// @param data The string data to parse.
        /// @param n The length of our string data.
        /// @return std::optional<Request> The parsed Request object, or std::nullopt if parsing fails.
        [[nodiscard]] static auto create(server::SharedSocket socket, const char *data, std::size_t n) -> std::optional<Request>;

        /// @brief Access a form value by key
        /// @param key The key of the form value to access
        /// @return std::optional<std::string> The value of the form data, or std::nullopt if the key is not found.
        [[nodiscard]] auto form(auto &&key) const -> std::optional<std::string_view> {
            if (auto it = forms.find(key); it != forms.end())
                return it->second;
            else
                return {};
        }

        /// @brief Accesses a header value by key.
        /// @param key The key of the header to access.
        /// @return std::optional<std::string> The value of the header, or std::nullopt if the key is not found.
        [[nodiscard]] auto header(auto &&key) const -> std::optional<std::string_view> {
            if (auto it = headers.find(key); it != headers.end())
                return it->second;
            else
                return {};
        }

        Route route;                ///< Trie routing data if it exists
        http::Method method;        ///< The HTTP method of the request
        request::Headers headers{}; ///< The headers of the request
        request::Headers forms{};   ///< The parsed form of the request
        std::string_view data{};    ///< The full data of the request
        std::string_view path{};    ///< The path of the request
        std::string_view body{};    ///< The body of the request
        server::SharedSocket socket;///< The underlying socket connection
    };

    auto Request::create(server::SharedSocket socket, const char *data, std::size_t n) -> std::optional<Request> {
        using namespace request::detail;

        // Initialize llparse
        llhttp_settings_t settings;
        llhttp_settings_init(&settings);
        settings.on_url             = on_url;
        settings.on_method_complete = on_method_complete;
        settings.on_header_field    = on_header_field;
        settings.on_header_value    = on_header_value;
        settings.on_body            = on_body;

        llhttp_t parser;
        llhttp_init(&parser, HTTP_REQUEST, &settings);

        // Store the RequestData object inside llparse for use in the callbacks
        RequestData req_data;
        parser.data = static_cast<void *>(&req_data);

        // Execute HTTP parser
        enum llhttp_errno err = llhttp_execute(&parser, data, n);
        if (err != HPE_OK && err != HPE_PAUSED_UPGRADE) {
            log::warn("Parse error: {} {}", llhttp_errno_name(err), parser.reason);
            return {};
        }

        // Validate the HTTP request
        Request req;

        // URL path must be non empty
        if (req_data.path.empty())
            return {};

        // Must have exactly the same number of header keys as header values
        if (req_data.keys.size() != req_data.values.size())
            return {};

        // Set the HTTP full data
        req.data = std::string_view(data, n);

        // Set the HTTP url path
        req.path = std::string_view(req_data.path.begin(), req_data.path.end());

        // Set the HTTP body
        req.body = std::string_view(req_data.data.begin(), req_data.data.end());

        // Set the HTTP method
        req.method = req_data.method;

        // Assemble Request headers from returned callback data
        for (std::size_t i = 0; i < req_data.keys.size(); i++) {
            const auto k   = std::string_view(req_data.keys[i].begin(), req_data.keys[i].end());
            const auto v   = std::string_view(req_data.values[i].begin(), req_data.values[i].end());
            req.headers[k] = v;
        }

        // Parse form data if the method is POST
        if (req.method == http::Method::POST) {
            req.forms = FormDataParser(req.data).parse();
        }

        // Assign underlying socket
        req.socket = socket;

        return req;
    }

}// namespace harbour