#include "cache.h"// initialize the cache at startup
#include "server.h"
#include "logger.h"
#include "middleware.h"

// Test out a raw response
auto TestHandler(Server &ctx, const Request &req, Response &resp) -> void {
    resp.set_type(ResponseType::Raw);
    resp.set_content("oh hey there");
}

// Echo the clients request back
auto EchoHandler(Server &ctx, const Request &req, Response &resp) -> void {
    resp.set_type(ResponseType::Ok);
    resp.set_header("Content-Type", "text/plain");
    resp.set_content(req.body);
}

auto main() -> int {
    Server server(8080);
    server.middleware(Middleware::Logger,
                      Middleware::FileServer,
                      Middleware::NotFound,
                      Middleware::DefaultIndex);
    server.route(
            Route{"/test", TestHandler},
            Route{"/echo", EchoHandler});
    server.serve();

    return 0;
}
