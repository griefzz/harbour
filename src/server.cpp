#include <format>
#include <string_view>
#include <string>

#include "cache.h"
#include "config.h"
#include "server.h"
#include "utility.h"
#include "socket/socket.h"

auto HttpRequest::parse(std::string_view req)
        -> std::expected<HttpRequest, HttpError> {
    HttpRequest result;

    // Find type
    size_t firstSpacePos = req.find(' ');
    if (firstSpacePos != std::string::npos) {
        auto type = req.substr(0, firstSpacePos);
        if (type == "GET") {
            result.type = HttpRequestType::GET;
        } else {
            return std::unexpected(HttpError::InternalServerError);
        }
    } else {
        return std::unexpected(HttpError::InternalServerError);
    }

    // Find path
    size_t secondSpacePos = req.find(' ', firstSpacePos + 1);
    if (secondSpacePos != std::string::npos) {
        result.path =
                req.substr(firstSpacePos + 1, secondSpacePos - firstSpacePos - 1);
    } else {
        return std::unexpected(HttpError::InternalServerError);
    }

    return result;
}

auto HttpResponseError() -> std::string {
    std::string_view msg = "Internal Server Error: The server encountered an "
                           "unexpected condition.";
    return std::format("HTTP/1.1 500 Internal Server Error\n"
                       "Date: {}\n"
                       "Server: {}/{}\n"
                       "Content-Type: text/plain\n"
                       "Content-Length: {}\n\n"
                       "{}",
                       getCurrentHttpDate(), ServerName, ServerVersion,
                       msg.size(), msg);
}

auto HttpResponseOk(std::string_view file) -> std::string {
    return std::format("HTTP/1.1 200 OK\n"
                       "Date: {}\n"
                       "Server: {}/{}\n"
                       "Content-Type: text/html\n"
                       "Content-Length: {}\n\n"
                       "{}",
                       getCurrentHttpDate(), ServerName, ServerVersion,
                       file.size(), file);
}

auto Server::request_handler(std::string_view data) -> std::string {
    if (auto req = HttpRequest::parse(data); req.has_value()) {
        // Handle default index
        if (req->path == "/") {
            if (auto index = cache["index.html"]; index.has_value()) {
                return HttpResponseOk(index.value());
            } else {
                return HttpResponseError();
            }
        }

        // Handle requested path
        auto path = fs::path(req->path).filename();
        if (auto content = cache[path]; content.has_value()) {
            return HttpResponseOk(content.value());
        } else {// File not found 404
            return HttpResponseError();
        }

        // Handle parsing errors
    } else if (req.error() == HttpError::InternalServerError) {
        return HttpResponseError();
    } else {
        return HttpResponseError();
    }
}

auto Server::serve() -> void {
    start_server(port, request_handler);
}
