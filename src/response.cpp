#include <format>
#include <harbour/response.hpp>

auto Status_string(Status type) -> std::string_view {
    switch (type) {
        case Status::Ok:
            return "200 OK";
        case Status::Unauthorized:
            return "401 Unauthorized";
        case Status::NotFound:
            return "404 Not Found";
        case Status::InternalServerError:
            return "500 Internal Server Error";
        case Status::NotImplemented:
            return "501 Not Implemented";
        default:
            return "418 I'm a teapot";// when all else fails
    }
}

auto Response::decode() noexcept -> std::string {
    // determine content type
    if (type == Status::Raw) {
        return content;
    }

    if (type == Status::InternalServerError) {
        set_header("Content-Type", "text/plain");
        set_content("Internal Server Error: The server encountered an "
                    "unexpected condition.");
    }

    if (type == Status::NotImplemented) {
        set_header("Content-Type", "text/plain");
        set_content("Not Implemented: The server does not implement the "
                    "requested method.");
    }

    if (type == Status::Unauthorized) {
        set_header("WWW-Authenticate", "Basic realm=\"Access to staging site\"");
    }

    set_header("Server", ServerName() + '/' + ServerVersion());

    std::string resp;
    resp += std::format("HTTP/1.1 {}\n", Status_string(type));
    for (const auto &entry: header) {
        resp += std::format("{}: {}\n", entry.first, entry.second);
    }

    if (!content.empty()) {
        resp += std::format("Content-Length: {}\n\n{}", content.size(), content);
    }

    return resp;
}
