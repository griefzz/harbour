#pragma once
#include <string_view>
#include <string>
#include <unordered_map>
#include <format>
#include <concepts>
#include <harbour/result.hpp>
#include <harbour/jsonable.hpp>
#include <harbour/config.hpp>

/// @brief Constructible object to create an raw TCP Response
struct Raw {
    Raw(std::string content) : content(std::move(content)) {}
    std::string content;
};

/// @brief Constructible object to create a plain/text Response
struct Plain {
    Plain(std::string content) : content(std::move(content)) {}
    std::string content;
};

/// @brief Concept that applies to all types convertable to a string_view
template<typename T>
concept StringLike = requires(T a) {
    { std::string_view{a} } -> std::convertible_to<std::string_view>;
};

/// @brief Constructible object to create an HTML Response
/// @tparam T Stringlike object
template<StringLike T>
struct Html {
    explicit constexpr Html(T &&content) noexcept : content(std::move(content)) {}
    explicit constexpr Html(const T &content) noexcept : content(std::move(content)) {}
    std::string_view content;
};


/// @brief Constructible object to create a JSON Response
/// @tparam T Jsonable object
template<Jsonable T>
struct Json {
    explicit constexpr Json(T &&t) noexcept : content(std::move(t.json())) {}
    explicit constexpr Json(const T &t) noexcept : content(std::move(t.json())) {}
    std::string content;
};

/// @brief Status of an HTTP Response
enum class Status {
    Raw                 = 0,
    Ok                  = 200,
    Unauthorized        = 401,
    NotFound            = 404,
    InternalServerError = 500,
    NotImplemented      = 501
};

/// @brief Convert a Status enum to a string_view
/// @param type Status to stringify
/// @return Stringified Status
auto Status_string(Status type) -> std::string_view;

// A response to be sent to clients
// use decode to create a raw buffer to send
struct Response {
    /// @brief Create an empty Response
    Response() noexcept = default;

    /// @brief Create a raw TCP Response
    /// @param raw The raw data to send to the client
    Response(const Raw &raw) noexcept {
        set_status(Status::Ok);
        set_content(raw.content);
    }

    /// @brief Create a plain/text HTTP Response
    /// @param plain The plain text data to send to the client
    Response(const Plain &plain) noexcept {
        set_status(Status::Ok);
        set_header("Content-Type", "text/plain");
        set_content(plain.content);
    }

    /// @brief Create an HTML HTTP Response
    /// @param html The html data to send to the client
    template<StringLike T>
    Response(Html<T> &&html) noexcept {
        set_status(Status::Ok);
        set_header("Content-Type", "text/html");
        set_content(std::move(html.content));
    }

    /// @brief Create a response from a Jsonable object
    /// @tparam T Jsonable struct
    /// @param json Object to jsonify
    template<Jsonable T>
    Response(Json<T> &&json) noexcept {
        set_status(Status::Ok);
        set_header("Content-Type", "application/json");
        set_content(std::move(json.content));
    }

    /// @brief Create an HTTP Response from a Status
    /// @param type The status of the HTTP Response
    Response(Status type) noexcept : type(type) {}

    /// @brief Create an HTTP Response from a Status with content
    /// @param type The status of the HTTP Response
    /// @param content The content body to send to the client
    Response(Status type, std::string_view content) noexcept : type(type), content(content) {}

    /// @brief Response type 200, 404 etc
    Status type;

    /// @brief Key value store for all HTTP response header values
    std::unordered_map<std::string_view, std::string> header;

    /// @brief Optional content to send to the client
    std::string content;

    /// @brief Set the status type of our Response
    /// @param t Status to set
    auto set_status(Status t) noexcept -> void { type = t; }

    /// @brief Set a custom header value
    /// @param key Key for our header
    /// @param val Value for our header
    auto set_header(std::string_view key, std::string_view val) noexcept -> void { header[key] = val; }

    /// @brief Convenience wrapper for setting header values
    /// @param key Key for our header
    /// @return Reference to our stored header value
    auto operator[](std::string_view key) noexcept -> std::string & { return header[key]; }

    /// @brief Set the content for our header
    /// @param c Content to set in our REsponse
    auto set_content(std::string_view c) noexcept -> void { content = c; }

    /// @brief Decode a response into a raw http header.
    /// @return Raw decoded HTTP Response to send to client
    auto decode() noexcept -> std::string;
};