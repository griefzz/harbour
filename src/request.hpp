#pragma once
#include <string_view>
#include <string>
#include <unordered_map>
#include <optional>
#include "result.hpp"
#include "forms.hpp"

// Error types associated with decoding an HTTP request
enum class RequestError {
    Unsupported,
    Invalid,
};

// Accepted request methods
enum class RequestMethod {
    GET,
    POST
};

// A request object recieved from a user
// use encode to create the Request object type
struct Request {
    // The HTTP request method
    RequestMethod method;

    // The path requested on the server
    std::string path;

    // Key value store for header information
    std::unordered_map<std::string, std::string> headers;

    // The entire raw request body
    std::string body;

    // Form data
    Form form;

    // Get the specified header value from key if it exists
    auto get_header(const std::string &key) const noexcept -> std::optional<std::string> {
        if (auto value = headers.find(key); value != headers.end()) {
            return value->second;
        } else {
            return {};
        }
    }

    // Convenience overload for get_header()
    auto operator[](const std::string &key) const noexcept -> std::optional<std::string> { return get_header(key); }

    // Create an Request from raw request data
    static auto encode(std::string_view req) noexcept -> Result<Request, RequestError> {
        // dont accept requests larger than 1kB
        if (req.size() > 1024) {
            return Err(RequestError::Invalid);
        }

        Request request;
        request.body = std::string(req);

        std::istringstream iss(request.body, std::ios_base::in);
        std::string line;

        // Read the first line to get the HTTP method and path
        std::getline(iss, line);
        std::istringstream firstLineStream(line);
        std::string method, path;
        firstLineStream >> method >> path;

        if (method != "GET" && method != "POST") {
            return Err(RequestError::Unsupported);
        }

        // 256 character long path is insane
        if (path.size() > 256) {
            return Err(RequestError::Invalid);
        }

        if (method == "GET") request.method = RequestMethod::GET;
        if (method == "POST") request.method = RequestMethod::POST;

        request.path = path;

        while (std::getline(iss, line) && !line.empty()) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string key   = line.substr(0, pos);
                std::string value = line.substr(pos + 2);

                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);

                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                // strip all newline characters
                value.erase(std::remove(value.begin(), value.end(), '\n'), value.end());
                value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());

                request.headers.emplace(std::move(key), std::move(value));
            }
        }

        if (request.method == RequestMethod::POST) {
            request.form = Forms::parse(line);
        }

        return request;
    }
};
