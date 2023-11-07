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
#include "request.h"
#include "response.h"
#include "config.h"
#include "server.h"
#include "socket/socket.h"

using Handler = std::function<void(const Request &, Response &)>;

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

    // Port used for the server
    uint32_t port;

    // middlewares for the server
    std::vector<Handler> middlewares;

    // routes for the server
    std::unordered_map<fs::path, Handler> routes;
};

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
    auto request_handler = [&](std::string_view data) -> std::string {
        Response resp(ResponseType::InternalServerError);

        if (auto req = Request::encode(data); req.has_value()) {
            // Handle middleware
            for (auto handler: this->middlewares) {
                handler(*req, resp);
            }

            // Handle routes
            for (auto route: this->routes) {
                if (auto s = this->routes.find(req->path); s != this->routes.end()) {
                    s->second(*req, resp);
                    return resp.decode();
                }
            }
        } else if (req.error() == RequestError::Unsupported) {
            resp = Response(ResponseType::NotImplemented);
        }

        return resp.decode();
    };

    start_server(port, request_handler);
}

static Server server(8080);