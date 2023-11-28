#pragma once
#include <string_view>
#include <string>
#include <unordered_map>
#include <format>
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

enum class ResponseType {
    Raw                 = 0,
    Ok                  = 200,
    Unauthorized        = 401,
    NotFound            = 404,
    InternalServerError = 500,
    NotImplemented      = 501
};

// Convert a ResponseType enum to a string_view
auto rt2sv(ResponseType type) -> std::string_view {
    switch (type) {
        case ResponseType::Ok:
            return "200 OK";
        case ResponseType::Unauthorized:
            return "401 Unauthorized";
        case ResponseType::NotFound:
            return "404 Not Found";
        case ResponseType::InternalServerError:
            return "500 Internal Server Error";
        case ResponseType::NotImplemented:
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
        type = ResponseType::Ok;
        set_content(std::move(raw.body));
    }
    Response(const Plain &plain) {
        type                   = ResponseType::Ok;
        header["Content-Type"] = "text/plain";
        set_content(std::move(plain.body));
    }
    Response(const Html &html) {
        type                   = ResponseType::Ok;
        header["Content-Type"] = "text/html";
        set_content(std::move(html.body));
    }
    template<jsonableConcept T>
    Response(const Json<T> &json) {
        type                   = ResponseType::Ok;
        header["Content-Type"] = "application/json";
        set_content(std::move(json.body));
    }
    Response(ResponseType type) : type(type) {}
    Response(ResponseType type, std::string_view content) : type(type), content(content) {}

    // Response type 200, 404 etc
    ResponseType type;

    // Key value store for all HTTP response header values
    std::unordered_map<std::string_view, std::string> header;

    // Optional content to send to the client
    std::string content;

    // set the type of our Response
    auto set_type(ResponseType t) noexcept { type = t; }

    // set a custom header value
    auto set_header(std::string_view key, std::string_view val) noexcept -> void { header[key] = val; }

    auto operator[](std::string_view key) noexcept -> std::string & { return header[key]; }

    // Set the content for our header
    auto set_content(std::string_view c) noexcept -> void { content = c; }

    // Decode a response into a raw http header.
    auto decode() noexcept -> std::string {
        // determine content type
        if (type == ResponseType::Raw) {
            return content;
        }

        if (type == ResponseType::InternalServerError) {
            set_header("Content-Type", "text/plain");
            set_content("Internal Server Error: The server encountered an "
                        "unexpected condition.");
        }

        if (type == ResponseType::NotImplemented) {
            set_header("Content-Type", "text/plain");
            set_content("Not Implemented: The server does not implement the "
                        "requested method.");
        }

        if (type == ResponseType::Unauthorized) {
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