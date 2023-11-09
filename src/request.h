#pragma once
#include <string_view>
#include <string>
#include <expected>

enum class RequestError {
    Unsupported,
    Invalid,
};

enum class RequestType {
    GET
};

// A request object recieved from a user
// use encode to create the Request object type
struct Request {
    RequestType type;
    std::string_view path;
    std::string_view body;

    // Create an Request from raw request data
    static auto encode(std::string_view req) -> std::expected<Request, RequestError>;
};

auto Request::encode(std::string_view req)
        -> std::expected<Request, RequestError> {
    Request result;

    result.body = req;

    // Find type
    size_t first = req.find(' ');
    if (first != std::string::npos) {
        auto type = req.substr(0, first);
        if (type == "GET") {
            result.type = RequestType::GET;
        } else {
            return std::unexpected(RequestError::Unsupported);
        }
    } else {
        return std::unexpected(RequestError::Invalid);
    }

    // Find path
    size_t second = req.find(' ', first + 1);
    if (second != std::string::npos) {
        result.path = req.substr(first + 1, second - first - 1);
    } else {
        return std::unexpected(RequestError::Invalid);
    }

    return result;
}
