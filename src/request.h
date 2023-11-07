#pragma once
#include <string_view>
#include <string>
#include <expected>

enum class RequestError {
    Unsupported,
    Invalid,
};

enum class RequestType {
    GET,
    HEAD
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

    // Find type
    size_t firstSpacePos = req.find(' ');
    if (firstSpacePos != std::string::npos) {
        auto type = req.substr(0, firstSpacePos);
        if (type == "GET") {
            result.type = RequestType::GET;
        } else if (type == "HEAD") {
            result.type = RequestType::HEAD;
        } else {
            return std::unexpected(RequestError::Unsupported);
        }
    } else {
        return std::unexpected(RequestError::Invalid);
    }

    // Find path
    size_t secondSpacePos = req.find(' ', firstSpacePos + 1);
    if (secondSpacePos != std::string::npos) {
        result.path =
                req.substr(firstSpacePos + 1, secondSpacePos - firstSpacePos - 1);
    } else {
        return std::unexpected(RequestError::Invalid);
    }

    // Parsed successfully, transfer body to Request
    result.body = req;

    return result;
}
