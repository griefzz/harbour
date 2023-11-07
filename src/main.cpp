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
    auto path = fs::path(req.path).filename();
    if (auto content = cache[path]; content.has_value()) {
        return HttpResponseOk(content.value());
    } else {// File not found 404
        return HttpResponseError();
    }
}

auto main() -> int {
    Server server(8080);
    server.middleware(default_index, file_server);
    //server.route("/test", test_handler);
    server.serve();

    return 0;
}
