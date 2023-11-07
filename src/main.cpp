#include "cache.h"// initialize the cache at startup
#include "server.h"

// Handle favicons
auto favicon(const Request &req, Response &resp) -> void {
    // is our request a route?
    if (auto s = server.routes.find(req.path); s != server.routes.end()) {
        return;
    }

    auto path = fs::path(req.path).filename();
    if (path == "favicon.ico") {
        if (auto content = cache[path]; content.has_value()) {
            resp.set_type(ResponseType::Ok);
            resp.set_header("Content-Type", "image/x-icon");
            resp.set_content(content.value());
        } else {
            resp.set_type(ResponseType::InternalServerError);
        }
    }
}

// Handle requested path
auto file_server(const Request &req, Response &resp) -> void {
    // is our request a route?
    if (auto s = server.routes.find(req.path); s != server.routes.end()) {
        return;
    }

    auto path = fs::path(req.path).filename();
    if (auto content = cache[path]; content.has_value()) {
        resp.set_type(ResponseType::Ok);
        resp.set_content(content.value());
    }

    // 404 file not found
    resp = Response(ResponseType::NotFound);
}

// Handle file not found
auto not_found(const Request &req, Response &resp) -> void {
    // is our request a route?
    if (auto s = server.routes.find(req.path); s != server.routes.end()) {
        return;
    }

    auto path = fs::path(req.path).filename();
    if (auto content = cache[path]; !content.has_value()) {
        if (auto file = cache["404.html"]; file.has_value()) {
            resp.set_type(ResponseType::NotFound);
            resp.set_header("Content-Type", "text/html");
            resp.set_content(file.value());
        } else {
            resp = Response(ResponseType::InternalServerError);
        }
    }
}

// Handle default index
auto index_handler(const Request &req, Response &resp) -> void {
    if (req.path == "/") {
        if (auto index = cache["index.html"]; index.has_value()) {
            resp.set_type(ResponseType::Ok);
            resp.set_header("Content-Type", "text/html");
            resp.set_content(*index);
        } else {
            resp = Response(ResponseType::InternalServerError);
        }
    }
}

// Handle test path
auto test_handler(const Request &req, Response &resp) -> void {
    resp.set_type(ResponseType::Raw);
    resp.set_content("oh hey there");
}

// Handle echo path
auto echo_handler(const Request &req, Response &resp) -> void {
    resp.set_type(ResponseType::Ok);
    resp.set_header("Content-Type", "text/plain");
    resp.set_content(req.body);
}

auto main() -> int {
    server.middleware(file_server, not_found, favicon);
    server.route(
            Route{"/", index_handler},
            Route{"/test", test_handler},
            Route{"/echo", echo_handler});
    server.serve();

    return 0;
}
