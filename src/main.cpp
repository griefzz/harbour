#include <harbour/harbour.hpp>
#include <harbour/middleware.hpp>
#include <harbour/handlers.hpp>
#include <harbour/stringable.hpp>
#include <harbour/jsonable.hpp>

// Test out a raw response
auto TestHandler(Server &ctx, const Request &req) -> Response {
    return Raw("oh hey there");
}

// Echo the clients request back
auto EchoHandler(Server &ctx, const Request &req) -> Response {
    return Plain(req.body);
}

// Serializable, from_formable and formattable Person
struct Person {
    std::string name;
    int age;

    HARBOUR_STRINGABLE(Person, name, age);
    HARBOUR_FROM_FORM(Person, name, age);
    HARBOUR_JSONABLE(name, age);
};
HARBOUR_FORMATTABLE(Person);

// Deserialize a Person and send to the client
auto PersonHandler(Server &ctx, const Request &req) -> Response {
    if (req.method == Method::POST) {
        if (auto p = Person::from_form(req.form)) {
            Logger::info(std::format("Client sent: {}", *p));
            return Json(*p);
        } else {
            Logger::warning("Unable to deserialize a Person!");
            return Status::InternalServerError;
        }
    } else {
        return Status::InternalServerError;
    }
}

// Deserialize a Person from an API and send to the client
auto ApiHandler(Server &ctx, const Request &req) -> Response {
    if (req.route.size() == 2) {
        auto p = Person{
                req["name"].value_or("nil"),
                std::stoi(req["age"].value_or("0"))};
        return Json(p);
        Logger::info(std::format("Client sent: {}", p));
    } else {
        Logger::warning("Unable to deserialize a Person!");
        return Status::InternalServerError;
    }
}

auto JsonHandler(Server &ctx, const Request &req) -> Response {
    struct JsonExample {
        std::string a              = "test123";
        std::vector<int> b         = {1, 2, 3};
        std::vector<float> c       = {1.1, 2.2, 3.3};
        std::vector<std::string> d = {"test", "1", "23"};
        Person e{"Bob", 23};

        HARBOUR_JSONABLE(a, b, c, d, e);
    };
    return Json(JsonExample{});
}

auto main() -> int {
    Server server;
    server.middleware(Http::Logger,
                      Http::FileServer,
#if HARBOUR_ENABLE_COMPRESSION
                      Http::Compression,
#endif
                      Http::DefaultIndex,
                      Http::NotFound);
    server.route(
            Route("/test", TestHandler),
            Route("/echo", EchoHandler),
            Route("/person", PersonHandler),
            Route("/json", JsonHandler),
            Route("/api/<name>/<age>/", ApiHandler),
            Route("/auth", Http::RequireAuth("admin:admin", Http::ServeFile("/index.html"))));
    server.serve();

    return 0;
}
