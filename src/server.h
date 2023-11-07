#pragma once
#include <expected>
#include <string_view>
#include <string>
#include <concepts>
#include "defines.h"

enum class HttpError {
    InternalServerError = 500,
};

// Craft an 500 Internal Server Error reponse
auto HttpResponseError() -> std::string;

// Craft an 200 OK response
auto HttpResponseOk(std::string_view file) -> std::string;

struct HttpRequest {
    enum class HttpRequestType {
        GET,
        // Maybe handle POST?
    };

    HttpRequestType type;
    std::string_view path;

    // Create an HttpRequest from raw request data
    static auto parse(std::string_view req) -> std::expected<HttpRequest, HttpError>;
};

class Server {
public:
    Server() : port(80) {}
    Server(u32 port) : port(port){};

    // Takes in raw http request data and returns a response
    auto request_handler(std::string_view data) -> std::string;

    // Serve the http server
    auto serve() -> void;

private:
    // Port used for the server
    u32 port;
};
