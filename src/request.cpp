#include <algorithm>
#include <harbour/request.hpp>
#include <harbour/result.hpp>
#include <harbour/forms.hpp>

auto Request::get_header(const std::string &key) const noexcept -> std::optional<std::string> {
    if (auto value = headers.find(key); value != headers.end()) {
        return value->second;
    } else {
        return {};
    }
}

auto Request::operator[](const std::string &key) const noexcept -> std::optional<std::string> {
    if (auto v = form.find(key); v != form.end()) {
        return v->second;
    }

    if (auto v = route.find(key); v != route.end()) {
        return v->second;
    }

    return {};
}

auto Request::encode(std::string_view req) noexcept -> Result<Request, RequestError> {
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

    if (method == "GET") request.method = Method::GET;
    if (method == "POST") request.method = Method::POST;

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

    if (request.method == Method::POST) {
        request.form = Forms::parse(line);
    }

    return request;
}