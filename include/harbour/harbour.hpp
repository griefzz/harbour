#pragma once
#include <vector>
#include <exception>
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

auto Server::get_route(const Request &req) noexcept -> std::optional<Route> {
    auto has_shared_root = [&](const auto &root) { return req.path == root.path || req.path.starts_with(root.root); };
    if (auto result = std::ranges::find_if(routes, has_shared_root); result != routes.end()) {
        return *result;
    }
    return {};
}

auto Server::serve() noexcept -> void {
    auto request_handler = [&](std::string_view data) -> std::string {
        Response resp(Status::InternalServerError);

        // We decode the request and pass it to our middleware
        // Then we determine if its a route and apply the route handler to the resp
        if (auto req = Request::encode(data)) {
            if (auto route = get_route(*req)) {
                if (auto map = route->parse(req->path)) {
                    req->route = *map;
                }
                resp = route->handler(*this, *req);
            }
            for (const auto &handler: middlewares) { handler(*this, *req, resp); }
        } else if (req.error() == RequestError::Unsupported) {
            Logger::info("User requested an unsupported method");
            resp = Response(Status::NotImplemented);
        } else if (req.error() == RequestError::Invalid) {
            Logger::warning(std::format("User sent an invalid request: {}", req->body));
        }

        return resp.decode();
    };

    try {
        start_server(port, request_handler);
    } catch (const std::exception &e) {
        Logger::error(e.what());
    }
}
