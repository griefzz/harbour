#pragma once
#include <string_view>
#include <string>
#include <unordered_map>
#include <optional>
#include "result.h"

// Error types associated with decoding an HTTP request
enum class RequestError {
    Unsupported,
    Invalid,
};

// Accepted request methods
enum class RequestMethod {
    GET
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

    // Get the specified header value from key if it exists
    auto get_header(const std::string &key) const -> std::optional<std::string> {
        if (auto value = headers.find(key); value != headers.end()) {
            return value->second;
        } else {
            return {};
        }
    }

    // Convenience overload for get_header()
    auto operator[](const std::string &key) const -> std::optional<std::string> { return get_header(key); }

    // Create an Request from raw request data
    static auto encode(std::string_view req) -> Result<Request, RequestError> {
        // dont accept requests larger than 10kB
        if (req.size() > 1024 * 10) {
            return Err(RequestError::Invalid);
        }

        Request request;
        request.body = req;

        std::istringstream iss(std::string(req), std::ios_base::in);
        std::string line;

        // Read the first line to get the HTTP method and path
        std::getline(iss, line);
        std::istringstream firstLineStream(line);
        std::string method, path;
        firstLineStream >> method >> path;

        if (method != "GET") {
            return Err(RequestError::Unsupported);
        }

        // 256 character long path is insane
        if (path.size() > 256) {
            return Err(RequestError::Invalid);
        }

        request.method = RequestMethod::GET;
        request.path   = path;

        // Read each subsequent line for headers
        while (std::getline(iss, line)) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string key   = line.substr(0, pos);
                std::string value = line.substr(pos + 1);

                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);

                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                request.headers[key] = value;
            }
        }

        return request;
    }
};
