#pragma once
#include <expected>
#include <string_view>
#include <string>
#include <concepts>
#include <format>
#include <string_view>
#include <string>
#include <optional>
#include <type_traits>
#include <unordered_map>

#include "cache.h"
#include "config.h"
#include "server.h"
#include "utility.h"
#include "socket/socket.h"

enum class HttpError {
    InternalServerError = 500,
};

// Craft an 500 Internal Server Error reponse
auto HttpResponseError() -> std::string;

// Craft an 200 OK response
auto HttpResponseOk(std::string_view file) -> std::string;

struct HttpRequest {
    enum class HttpRequestType {
        GET
    };

    HttpRequestType type;
    std::string_view path;

    // Create an HttpRequest from raw request data
    static auto parse(std::string_view req) -> std::expected<HttpRequest, HttpError>;
};

using Handler = std::function<std::optional<std::string>(const HttpRequest &)>;

struct Route {
    fs::path path;
    Handler handler;
};

struct Server {
    Server() : port(80) {}
    Server(uint32_t port) : port(port) {}

    template<typename... Args>
    auto middleware(Args &&...args) -> void;

    template<typename... Args>
    auto route(Args &&...args) -> void;

    // Serve the http server
    auto serve() -> void;

    // Handler used for serve
    auto request_handler(std::string_view data) -> std::string;

    // Port used for the server
    uint32_t port;

    // middlewares for the server
    std::vector<Handler> middlewares;

    // routes for the server
    std::unordered_map<fs::path, Handler> routes;
};

auto HttpRequest::parse(std::string_view req)
        -> std::expected<HttpRequest, HttpError> {
    HttpRequest result;

    // Find type
    size_t firstSpacePos = req.find(' ');
    if (firstSpacePos != std::string::npos) {
        auto type = req.substr(0, firstSpacePos);
        if (type == "GET") {
            result.type = HttpRequestType::GET;
        } else {
            return std::unexpected(HttpError::InternalServerError);
        }
    } else {
        return std::unexpected(HttpError::InternalServerError);
    }

    // Find path
    size_t secondSpacePos = req.find(' ', firstSpacePos + 1);
    if (secondSpacePos != std::string::npos) {
        result.path =
                req.substr(firstSpacePos + 1, secondSpacePos - firstSpacePos - 1);
    } else {
        return std::unexpected(HttpError::InternalServerError);
    }

    return result;
}

auto HttpResponseError() -> std::string {
    std::string_view msg = "Internal Server Error: The server encountered an "
                           "unexpected condition.";
    return std::format("HTTP/1.1 500 Internal Server Error\n"
                       "Date: {}\n"
                       "Server: {}/{}\n"
                       "Content-Type: text/plain\n"
                       "Content-Length: {}\n\n"
                       "{}",
                       getCurrentHttpDate(), ServerName, ServerVersion,
                       msg.size(), msg);
}

auto HttpResponseOk(std::string_view file) -> std::string {
    return std::format("HTTP/1.1 200 OK\n"
                       "Date: {}\n"
                       "Server: {}/{}\n"
                       "Content-Type: text/html\n"
                       "Content-Length: {}\n\n"
                       "{}",
                       getCurrentHttpDate(), ServerName, ServerVersion,
                       file.size(), file);
}

auto Server::request_handler(std::string_view data) -> std::string {
    if (auto req = HttpRequest::parse(data); req.has_value()) {
        // Handle routes
        for (auto route: routes) {
            if (auto s = routes.find(req->path); s != routes.end()) {
                if (auto resp = s->second(*req)) {
                    return *resp;
                }
            }
        }

        // Handle middleware
        for (auto handler: middlewares) {
            if (auto resp = handler(*req)) {
                return *resp;
            }
        }
    }

    return HttpResponseError();
}

template<typename... Args>
auto Server::middleware(Args &&...args) -> void {
    static_assert((std::is_constructible_v<Handler, Args &&> && ...));
    (middlewares.push_back(std::forward<Args>(args)), ...);
}

template<typename... Args>
auto Server::route(Args &&...args) -> void {
    static_assert((std::is_constructible_v<Route, Args &&> && ...));
    ((routes[args.path] = args.handler), ...);
}

auto Server::serve() -> void {
    auto wrapper = [this](std::string_view v) -> std::string { return this->request_handler(v); };
    start_server(port, wrapper);
}

static Server server(ServerPort);