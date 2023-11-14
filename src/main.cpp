#include "server.h"
#include "middleware.h"
#include "handlers.h"

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

// Display our source code index
auto SrcIndexHandler(Server &ctx, const Request &req, Response &resp) -> void {
    if (auto index = ctx.cache["/src/index.html"]) {
        resp.set_type(ResponseType::Ok);
        //resp.set_header("Content-Type", "text/html");
        resp["Content-Type"] = "text/html";
        resp.set_content(*index);
    } else {
        resp = Response(ResponseType::InternalServerError);
    }
}

auto main() -> int {
    Server server(8080);
    server.middleware(Middleware::Logger,
                      Middleware::FileServer,
                      Middleware::NotFound,
                      Middleware::DefaultIndex);
    server.route(
            Route{"/", Handlers::ServeFile("/index.html")},
            Route{"/test", TestHandler},
            Route{"/echo", EchoHandler},
            Route{"/src", SrcIndexHandler},
            Route{"/src/", SrcIndexHandler},
            Route{"/auth", Handlers::RequireAuth("admin:admin", Handlers::ServeFile("/index.html"))});
    server.serve();

    return 0;
}
