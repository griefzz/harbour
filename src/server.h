#pragma once
#include <string_view>
#include <string>
#include <concepts>
#include <format>
#include <string_view>
#include <string>
#include <optional>
#include <type_traits>
#include <unordered_map>

#include "logger.h"
#include "cache.h"
#include "request.h"
#include "response.h"
#include "config.h"
#include "server.h"
#include "socket/socket.h"

struct Server;
using Handler = std::function<void(Server &ctx, const Request &, Response &)>;

template<typename T>
concept HandlerConcept = requires(T handler, Server &ctx, const Request &req, Response &res) {
    { handler(ctx, req, res) } -> std::same_as<void>;
};

struct Route {
    fs::path path;
    Handler handler;
};

template<typename T>
concept RouteConcept = requires(T t) {
    { t } -> std::same_as<Route>;
};

struct Server {
    Server() : port(80) {}
    Server(uint32_t port) : port(port) {}

    // Include middleware in the server
    template<HandlerConcept... M>
    auto middleware(M &&...m) -> void;

    // Include a route in the server: Route{"/path", Handler}
    template<typename... R>
    auto route(R &&...r) -> void;

    // Serve the http server
    auto serve() -> void;

    // Determine if a request is a route
    auto is_route(const Request &req) -> bool;

    // Port used for the server
    uint32_t port;

    // File cache
    Cache cache;

    // middlewares for the server
    std::vector<Handler> middlewares;

    // routes for the server
    std::unordered_map<fs::path, Handler> routes;
};

template<HandlerConcept... M>
auto Server::middleware(M &&...m) -> void {
    (middlewares.push_back(std::forward<M>(m)), ...);
}

template<typename... R>
auto Server::route(R &&...r) -> void {
    static_assert((std::is_constructible_v<Route, R &&> && ...));
    ((routes[r.path] = r.handler), ...);
}

auto Server::serve() -> void {
    auto request_handler = [&](std::string_view data) -> std::string {
        Response resp(ResponseType::InternalServerError);

        if (auto req = Request::encode(data); req.has_value()) {
            // Handle middleware
            for (auto handler: this->middlewares) {
                handler(*this, *req, resp);
            }

            // Handle routes
            for (auto route: this->routes) {
                if (auto s = this->routes.find(req->path); s != this->routes.end()) {
                    s->second(*this, *req, resp);
                    return resp.decode();
                }
            }
        } else if (req.error() == RequestError::Unsupported) {
            Logger::info("User requested an unsupported method");
            resp = Response(ResponseType::NotImplemented);
        } else if (req.error() == RequestError::Invalid) {
            Logger::warning(std::format("User sent an invalid request: {}", req->body));
        }

        return resp.decode();
    };

    start_server(port, request_handler);
}

auto Server::is_route(const Request &req) -> bool {
    return routes.find(req.path) != routes.end();
}