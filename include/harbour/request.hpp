#pragma once
#include <string_view>
#include <string>
#include <unordered_map>
#include <optional>
#include <harbour/result.hpp>
#include <harbour/forms.hpp>

using RouteMap = std::unordered_map<std::string, std::string>;

// Error types associated with decoding an HTTP request
enum class RequestError {
    Unsupported,
    Invalid,
};

// Accepted request methods
enum class Method {
    GET,
    POST
};

// A request object recieved from a user
// use encode to create the Request object type
struct Request {
    // The HTTP request method
    Method method;

    // The path requested on the server
    std::string path;

    // Key value store for header information
    std::unordered_map<std::string, std::string> headers;

    // The entire raw request body
    std::string body;

    // Form data
    Form form;

    // Route map data
    RouteMap route;

    // Get the specified header value from key if it exists
    auto get_header(const std::string &key) const noexcept -> std::optional<std::string>;

    // Convenience overload for getting Form data or RouteMap data
    auto operator[](const std::string &key) const noexcept -> std::optional<std::string>;

    // Create an Request from raw request data
    static auto encode(std::string_view req) noexcept -> Result<Request, RequestError>;
};
