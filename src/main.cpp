#include "cache.h"// initialize the cache at startup
#include "server.h"
#include "logger.h"
#include "middleware.h"

// Serve our index.html file when requesting /
auto index_handler(const Request &req, Response &resp) -> void {
    if (req.path == fs::path("/")) {
        if (auto index = cache["index.html"]; index.has_value()) {
            resp.set_type(ResponseType::Ok);
            resp.set_header("Content-Type", "text/html");
            resp.set_content(*index);
        } else {
            Logger::error("Unable to find index page!");
            resp = Response(ResponseType::InternalServerError);
        }
    }
}

// Test out a raw response
auto test_handler(const Request &req, Response &resp) -> void {
    resp.set_type(ResponseType::Raw);
    resp.set_content("oh hey there");
}

// Echo the clients request back
auto echo_handler(const Request &req, Response &resp) -> void {
    resp.set_type(ResponseType::Ok);
    resp.set_header("Content-Type", "text/plain");
    resp.set_content(req.body);
}

auto main() -> int {
    server.middleware(file_server, not_found);
    server.route(
            Route{"/", index_handler},
            Route{"/test", test_handler},
            Route{"/echo", echo_handler});
    server.serve();

    return 0;
}
