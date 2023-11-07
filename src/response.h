#pragma once
#include <string_view>
#include <string>
#include <unordered_map>
#include <format>

enum class ResponseType {
    Raw                 = 0,
    Ok                  = 200,
    NotFound            = 404,
    InternalServerError = 500,
};

auto rt2sv(ResponseType type) -> std::string_view {
    switch (type) {
        case ResponseType::Ok:
            return "200 OK";
        case ResponseType::NotFound:
            return "404 Not Found";
        case ResponseType::InternalServerError:
            return "500 Internal Server Error";
        default:
            return "418 I'm a teapot";// when all else fails
    }
}

struct Response {
    Response() {}
    Response(ResponseType type) : type(type) {}
    Response(ResponseType type, std::string_view content) : type(type), content(content) {}

    ResponseType type;
    std::unordered_map<std::string_view, std::string> header;
    std::string content;

    auto set_type(ResponseType t) { type = t; }

    auto set_header(std::string_view key, std::string_view val) -> void {
        header[key] = val;
    }

    auto set_content(std::string_view c) -> void { content = c; }

    auto decode() -> std::string {
        // determine content type
        if (type == ResponseType::Raw) {
            return content;
        }
        if (type == ResponseType::InternalServerError) {
            set_header("Content-Type", "text/plain");
            set_content("Internal Server Error: The server encountered an "
                        "unexpected condition.");
        }

        std::string resp;
        resp += std::format("HTTP/1.1 {}\n", rt2sv(type));
        for (const auto &entry: header) {
            resp += std::format("{}: {}\n", entry.first, entry.second);
        }

        if (!content.empty()) {
            resp += std::format("Content-Length: {}\n\n{}", content.size(),
                                content);
        }
        return resp;
    }
};