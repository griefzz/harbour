#pragma once
#include <string_view>
#include <string>
#include <unordered_map>
#include <format>
#include <harbour/result.hpp>
#include "config.hpp"

struct Raw {
    Raw(const std::string &body) : body(std::move(body)) {}
    std::string body;
};

struct Plain {
    Plain(const std::string &body) : body(std::move(body)) {}
    std::string body;
};

struct Html {
    Html(const std::string &body) : body(std::move(body)) {}
    std::string body;
};

template<typename T>
concept jsonableConcept = requires(T x) {
    { x.json() } -> std::same_as<std::string>;
};

template<jsonableConcept T>
struct Json {
    Json(T t) : body(std::move(t.json())) {}
    std::string body;
};

enum class Status {
    Raw                 = 0,
    Ok                  = 200,
    Unauthorized        = 401,
    NotFound            = 404,
    InternalServerError = 500,
    NotImplemented      = 501
};

// Convert a ResponseType enum to a string_view
auto rt2sv(Status type) -> std::string_view {
    switch (type) {
        case Status::Ok:
            return "200 OK";
        case Status::Unauthorized:
            return "401 Unauthorized";
        case Status::NotFound:
            return "404 Not Found";
        case Status::InternalServerError:
            return "500 Internal Server Error";
        case Status::NotImplemented:
            return "501 Not Implemented";
        default:
            return "418 I'm a teapot";// when all else fails
    }
}

// A response to be sent to clients
// use decode to create a raw buffer to send
struct Response {
    Response() {}

    Response(const Raw &raw) {
        type = Status::Ok;
        set_content(std::move(raw.body));
    }

    Response(const Plain &plain) {
        type                   = Status::Ok;
        header["Content-Type"] = "text/plain";
        set_content(std::move(plain.body));
    }

    Response(const Html &html) {
        type                   = Status::Ok;
        header["Content-Type"] = "text/html";
        set_content(std::move(html.body));
    }

    template<jsonableConcept T>
    Response(const Json<T> &json) {
        type                   = Status::Ok;
        header["Content-Type"] = "application/json";
        set_content(std::move(json.body));
    }

    Response(Status type) : type(type) {}

    Response(Status type, std::string_view content) : type(type), content(content) {}

    template<typename T = Response, typename E = Status>
    Response(const Result<T, E> result) {
        if (auto r = result.value(); result.has_value()) {
            *this = r;
        } else {
            *this = result.error();
        }
    }

    // Response type 200, 404 etc
    Status type;

    // Key value store for all HTTP response header values
    std::unordered_map<std::string_view, std::string> header;

    // Optional content to send to the client
    std::string content;

    // Set the status type of our Response
    auto set_status(Status t) noexcept { type = t; }

    // Set a custom header value
    auto set_header(std::string_view key, std::string_view val) noexcept -> void { header[key] = val; }

    // Convenience wrapper for setting header values
    auto operator[](std::string_view key) noexcept -> std::string & { return header[key]; }

    // Set the content for our header
    auto set_content(std::string_view c) noexcept -> void { content = c; }

    // Decode a response into a raw http header.
    auto decode() noexcept -> std::string {
        // determine content type
        if (type == Status::Raw) {
            return content;
        }

        if (type == Status::InternalServerError) {
            set_header("Content-Type", "text/plain");
            set_content("Internal Server Error: The server encountered an "
                        "unexpected condition.");
        }

        if (type == Status::NotImplemented) {
            set_header("Content-Type", "text/plain");
            set_content("Not Implemented: The server does not implement the "
                        "requested method.");
        }

        if (type == Status::Unauthorized) {
            set_header("WWW-Authenticate", "Basic realm=\"Access to staging site\"");
        }

        set_header("Server", ServerName + '/' + ServerVersion);

        std::string resp;
        resp += std::format("HTTP/1.1 {}\n", rt2sv(type));
        for (const auto &entry: header) {
            resp += std::format("{}: {}\n", entry.first, entry.second);
        }

        if (!content.empty()) {
            resp += std::format("Content-Length: {}\n\n{}", content.size(), content);
        }

        return resp;
    }
};