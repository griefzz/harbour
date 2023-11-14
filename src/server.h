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
concept RouteConcept = std::is_same_v<T, Route>;

struct Server {
    Server() : port(80) {}
    Server(uint32_t port) : port(port) {}

    // Include middleware in the server
    template<HandlerConcept... M>
    auto middleware(M &&...m) noexcept -> void;

    // Include a route in the server: Route{"/path", Handler}
    template<RouteConcept... R>
    auto route(R &&...r) noexcept -> void;

    // Determine if a requests path is a route
    auto is_route(const Request &req) noexcept -> bool;

    // Serve the http server
    auto serve() noexcept -> void;

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
auto Server::middleware(M &&...m) noexcept -> void {
    (middlewares.push_back(std::forward<M>(m)), ...);
}

template<RouteConcept... R>
auto Server::route(R &&...r) noexcept -> void {
    ((routes[r.path] = r.handler), ...);
}

auto Server::is_route(const Request &req) noexcept -> bool {
    return routes.contains(req.path);
}

auto Server::serve() noexcept -> void {
    auto request_handler = [&](std::string_view data) -> std::string {
        Response resp(ResponseType::InternalServerError);

        // We decode the request and pass it to our middleware
        // Then we determine if its a route and apply the route handler to the resp
        if (auto req = Request::encode(data)) {
            for (auto handler: middlewares) { handler(*this, *req, resp); }
            if (is_route(*req)) { routes[req->path](*this, *req, resp); }
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
