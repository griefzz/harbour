#pragma once
#include <harbour/logger.hpp>
#include <harbour/request.hpp>
#include <harbour/response.hpp>
#include <harbour/config.hpp>
#include "cache.hpp"
#include "socket/socket.hpp"
#include "route.hpp"

struct Server;
template<typename T>
concept MiddlewareConcept = requires(T middleware, Server &ctx, const Request &req, Response &res) {
    { middleware(ctx, req, res) } -> std::same_as<void>;
};

struct Server {
    explicit Server(uint32_t port = ServerPort) : port(port) {}

    // Include middleware in the server
    template<MiddlewareConcept... M>
    auto middleware(M &&...m) noexcept -> void {
        (middlewares.push_back(std::forward<M>(m)), ...);
    }

    // Include a route in the server: Route{"/path", Handler}
    template<RouteConcept... R>
    auto route(R &&...r) noexcept -> void {
        (routes.push_back(std::forward<R>(r)), ...);
    }

    // Get a Route from a Request if it exists
    auto get_route(const Request &req) noexcept -> std::optional<Route>;

    // Serve the http server
    auto serve() noexcept -> void;

    // Port used for the server
    uint32_t port;

    // File cache
    Cache cache;

    // middlewares for the server
    std::vector<Middleware> middlewares;

    // routes for the server
    std::vector<Route> routes;
};
