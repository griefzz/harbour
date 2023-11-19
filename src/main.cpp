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

// Deserialze a person and send to client
auto PersonHandler(Server &ctx, const Request &req, Response &resp) -> void {
    struct Person {
        std::string name;
        int age;

        auto string() -> std::string {
            return std::format("Person ( name: {}, age: {} )\n", name, age);
        }

        HARBOUR_DESERIALIZABLE(Person, age, name)
    };

    if (req.method == RequestMethod::POST) {
        if (auto p = Person::from_form(req.form)) {
            resp.set_type(ResponseType::Ok);
            resp.set_header("Content-Type", "text/plain");
            resp.set_content(p->string());
        } else {
            Logger::warning("Unable to deserialize a Person!");
        }
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
            Route{"/", Handlers::ServeFile("/index.html")},
            Route{"/test", TestHandler},
            Route{"/echo", EchoHandler},
            Route{"/person", PersonHandler},
            Route{"/auth", Handlers::RequireAuth("admin:admin", Handlers::ServeFile("/index.html"))});
    server.serve();

    return 0;
}
