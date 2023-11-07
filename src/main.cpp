#include "cache.h"// initialize the cache at startup
#include "server.h"

// Handle default index
auto default_index(const HttpRequest &req) -> std::optional<std::string> {
    if (req.path == "/") {
        if (auto index = cache["index.html"]; index.has_value()) {
            return HttpResponseOk(index.value());
        } else {
            return HttpResponseError();
        }
    }

    return {};
}

// Handle requested path
auto file_server(const HttpRequest &req) -> std::optional<std::string> {
    // is our request a route?
    if (auto s = server.routes.find(req.path); s != server.routes.end()) {
        return {};
    }

    auto path = fs::path(req.path).filename();
    if (auto content = cache[path]; content.has_value()) {
        return HttpResponseOk(content.value());
    }

    // 404 file not found
    return HttpResponseError();
}

// Handle test path
auto test_handler(const HttpRequest &req) -> std::optional<std::string> {
    return "oh hey there";
}

// Handle echo path
auto echo_handler(const HttpRequest &req) -> std::optional<std::string> {
    return req.path.data();
}

auto main() -> int {
    server.middleware(default_index,
                      file_server);
    server.route(
            Route{"/test", test_handler},
            Route{"/echo", echo_handler});
    server.serve();

    return 0;
}
