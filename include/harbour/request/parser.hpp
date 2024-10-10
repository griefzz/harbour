///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file parser.hpp
/// @brief Contains the implementation of harbours http request parser

#pragma once

#include <span>
#include <vector>

#include <llhttp.h>

#include "../http/method.hpp"

namespace harbour::request::detail {

    /// @brief Structure to hold request data.
    struct RequestData {
        std::span<const char> path;                 //< URL path for the parser callbacks
        std::span<const char> data;                 //< Request body for callbacks
        std::vector<std::span<const char>> keys{};  //< Keys returned from parser callbacks
        std::vector<std::span<const char>> values{};//< Values returned from parser callbacks
        http::Method method;                        //< Method returned from parser callbacks
    };

    /// @brief Callback function for URL parsing.
    /// @param p Pointer to the llhttp_t structure.
    /// @param at Pointer to the URL data.
    /// @param length Length of the URL data.
    /// @return HPE_OK on success.
    int on_url(llhttp_t *p, const char *at, size_t length) {
        auto req  = static_cast<RequestData *>(p->data);
        req->path = std::span(at, length);
        return HPE_OK;
    }

    /// @brief Callback function for header field parsing.
    /// @param p Pointer to the llhttp_t structure.
    /// @param at Pointer to the header field data.
    /// @param length Length of the header field data.
    /// @return HPE_OK on success.
    int on_header_field(llhttp_t *p, const char *at, size_t length) {
        auto req = static_cast<RequestData *>(p->data);
        req->keys.emplace_back(at, length);
        return HPE_OK;
    }

    /// @brief Callback function for header value parsing.
    /// @param p Pointer to the llhttp_t structure.
    /// @param at Pointer to the header value data.
    /// @param length Length of the header value data.
    /// @return HPE_OK on success.
    int on_header_value(llhttp_t *p, const char *at, size_t length) {
        auto req = static_cast<RequestData *>(p->data);
        req->values.emplace_back(at, length);
        return HPE_OK;
    }

    /// @brief Callback function for body parsing.
    /// @param p Pointer to the llhttp_t structure.
    /// @param at Pointer to the body data.
    /// @param length Length of the body data.
    /// @return HPE_OK on success.
    int on_body(llhttp_t *p, const char *at, size_t length) {
        auto req  = static_cast<RequestData *>(p->data);
        req->data = std::span(at, length);
        return HPE_OK;
    }

    /// @brief Callback function for method parsing.
    /// @param p Pointer to the llhttp_t structure.
    /// @return HPE_OK on success. HPE_INVALID_METHOD on failure
    int on_method_complete(llhttp_t *p) {
        auto req = static_cast<RequestData *>(p->data);
        switch (llhttp_get_method(p)) {
            case HTTP_GET:
                req->method = http::Method::GET;
                return HPE_OK;
            case HTTP_POST:
                req->method = http::Method::POST;
                return HPE_OK;
            case HTTP_PUT:// TODO
                req->method = http::Method::PUT;
                return HPE_INVALID_METHOD;
            case HTTP_HEAD:// TODO
                req->method = http::Method::HEAD;
                return HPE_INVALID_METHOD;
            default:
                return HPE_INVALID_METHOD;
        }
    }

}// namespace harbour::request::detail