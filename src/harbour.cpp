#include <harbour/harbour.hpp>

auto Server::get_route(const Request &req) noexcept -> std::optional<Route> {
    auto has_shared_root = [&](const auto &r) { return req.path == r.path || req.path.starts_with(r.root); };
    if (auto result = std::ranges::find_if(routes, has_shared_root); result != routes.end()) {
        return *result;
    } else {
        return {};
    }
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
            for (auto handler: middlewares) { handler(*this, *req, resp); }
        } else if (req.error() == RequestError::Unsupported) {
            Logger::info("User requested an unsupported method");
            resp = Response(Status::NotImplemented);
        } else if (req.error() == RequestError::Invalid) {
            Logger::warning(std::format("User sent an invalid request: {}", req->body));
        }

        return resp.decode();
    };

    start_server(port, request_handler);
}
