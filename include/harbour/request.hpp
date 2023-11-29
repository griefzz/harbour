/// @file request.hpp
/// @brief Defines the Request structure and associated enums for handling HTTP requests.
#pragma once
#include <string_view>
#include <string>
#include <unordered_map>
#include <optional>
#include <harbour/result.hpp>
#include <harbour/forms.hpp>

/// Type alias for a map that associates route keys with route values.
using RouteMap = std::unordered_map<std::string, std::string>;

/// Enum class representing the types of errors that can occur when decoding an HTTP request.
enum class RequestError {
    Unsupported,///< The request method is not supported.
    Invalid,    ///< The request is invalid.
};

/// Enum class representing the accepted HTTP request methods.
enum class Method {
    GET,///< Represents an HTTP GET request.
    POST///< Represents an HTTP POST request.
};

/// @struct Request
/// @brief Represents an HTTP request received from a user.
///
/// This structure contains all the necessary components of an HTTP request,
/// including the method, path, headers, body, form data, and route map data.
struct Request {
    /// The HTTP request method.
    Method method;

    /// The path requested on the server.
    std::string path;

    /// Key-value store for header information.
    std::unordered_map<std::string, std::string> headers;

    /// The entire raw request body.
    std::string body;

    /// Form data parsed from the request body.
    Form form;

    /// Route map data extracted from the request path.
    RouteMap route;

    /// @brief Get the specified header value from key if it exists.
    /// @param key The header key to search for.
    /// @return An optional string containing the header value if found.
    auto get_header(const std::string &key) const noexcept -> std::optional<std::string>;

    /// @brief Convenience overload for getting Form data or RouteMap data.
    /// @param key The key to search for in Form data or RouteMap data.
    /// @return An optional string containing the value if found.
    auto operator[](const std::string &key) const noexcept -> std::optional<std::string>;

    /// @brief Create a Request object from raw request data.
    /// @param req The raw HTTP request data as a string view.
    /// @return A Result object containing either a Request on success or a RequestError on failure.
    static auto encode(std::string_view req) noexcept -> Result<Request, RequestError>;
};