#include "server.hpp"
#include "middleware.hpp"
#include "handlers.hpp"

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
#if SERVER_ENABLE_COMPRESSION
                      Middleware::Compression,
#endif
                      Middleware::DefaultIndex,
                      Middleware::NotFound);
    server.route(
            Route{"/", Handlers::ServeFile("/index.html")},
            Route{"/test", TestHandler},
            Route{"/echo", EchoHandler},
            Route{"/auth", Handlers::RequireAuth("admin:admin", Handlers::ServeFile("/index.html"))});
    server.serve();

    return 0;
}
