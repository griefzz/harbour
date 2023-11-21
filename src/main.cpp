#include "server.hpp"
#include "middleware.hpp"
#include "handlers.hpp"
#include "stringable.h"

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

// Serializable, from_formable and formattable Person
struct Person {
    std::string name;
    int age;

    HARBOUR_STRINGABLE(Person, name, age);
    HARBOUR_FROM_FORM(Person, name, age);
};
HARBOUR_FORMATTABLE(Person);

// Deserialize a Person and send to the client
auto PersonHandler(Server &ctx, const Request &req, Response &resp) -> void {
    if (req.method == RequestMethod::POST) {
        if (auto p = Person::from_form(req.form)) {
            resp.set_type(ResponseType::Ok);
            resp.set_header("Content-Type", "text/plain");
            resp.set_content(p->string());
            Logger::info(std::format("Client sent: {}", *p));
        } else {
            Logger::warning("Unable to deserialize a Person!");
            resp = Response(ResponseType::InternalServerError);
        }
    }
}

// Deserialize a Person from an API and send to the client
auto ApiHandler(Server &ctx, const Request &req, Response &resp) -> void {
    if (req.route.size() == 2) {
        Person p;
        p.name = req["name"].value_or("nil");
        p.age  = std::stoi(req["age"].value_or("0"));
        resp.set_type(ResponseType::Ok);
        resp.set_header("Content-Type", "text/plain");
        resp.set_content(p.string());
        Logger::info(std::format("Client sent: {}", p));
    } else {
        Logger::warning("Unable to deserialize a Person!");
        resp = Response(ResponseType::InternalServerError);
    }
}

auto main() -> int {
    Server server(8080);
    server.middleware(Middleware::Logger,
                      Middleware::FileServer,
#if HARBOUR_ENABLE_COMPRESSION
                      Middleware::Compression,
#endif
                      Middleware::DefaultIndex,
                      Middleware::NotFound);
    server.route(
            Route("/test", TestHandler),
            Route("/echo", EchoHandler),
            Route("/person", PersonHandler),
            Route("/api/<name>/<age>/", ApiHandler),
            Route("/auth", Handlers::RequireAuth("admin:admin", Handlers::ServeFile("/index.html"))));
    server.serve();

    return 0;
}
