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
        if (auto v = req["Host"]) Logger::info(std::format("Host: {}\n", *v));
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
            Route{"/test", TestHandler},
            Route{"/echo", EchoHandler},
            Route{"/src", SrcIndexHandler},
            Route{"/src/", SrcIndexHandler},
            Route{"/awd", Handlers::ServeFile("/a.html")});
    server.serve();

    return 0;
}
