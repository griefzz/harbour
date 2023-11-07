#pragma once
#include <string_view>
#include <string>
#include <expected>

enum class RequestError {
    Invalid,
};

struct Request {
    enum class RequestType {
        GET
    };

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
    size_t firstSpacePos = req.find(' ');
    if (firstSpacePos != std::string::npos) {
        auto type = req.substr(0, firstSpacePos);
        if (type == "GET") {
            result.type = RequestType::GET;
        } else {
            return std::unexpected(RequestError::Invalid);
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

    return result;
}
