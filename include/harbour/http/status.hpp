///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file status.hpp
/// @brief This file contains the implementation of harbours http status type

#pragma once

#include <iostream>

#include <fmt/base.h>
#include <fmt/format.h>

namespace harbour {
    namespace http {

        /// @brief Enum class representing HTTP status codes.
        enum class Status {
            Continue                      = 100,///< HTTP 100 Continue
            SwitchingProtocols            = 101,///< HTTP 101 Switching Protocols
            Processing                    = 102,///< HTTP 102 Processing
            OK                            = 200,///< HTTP 200 OK
            Created                       = 201,///< HTTP 201 Created
            Accepted                      = 202,///< HTTP 202 Accepted
            NonAuthoritativeInformation   = 203,///< HTTP 203 Non-Authoritative Information
            NoContent                     = 204,///< HTTP 204 No Content
            ResetContent                  = 205,///< HTTP 205 Reset Content
            PartialContent                = 206,///< HTTP 206 Partial Content
            MultiStatus                   = 207,///< HTTP 207 Multi-Status
            AlreadyReported               = 208,///< HTTP 208 Already Reported
            IMUsed                        = 226,///< HTTP 226 IM Used
            MultipleChoices               = 300,///< HTTP 300 Multiple Choices
            MovedPermanently              = 301,///< HTTP 301 Moved Permanently
            Found                         = 302,///< HTTP 302 Found
            SeeOther                      = 303,///< HTTP 303 See Other
            NotModified                   = 304,///< HTTP 304 Not Modified
            UseProxy                      = 305,///< HTTP 305 Use Proxy
            TemporaryRedirect             = 307,///< HTTP 307 Temporary Redirect
            PermanentRedirect             = 308,///< HTTP 308 Permanent Redirect
            BadRequest                    = 400,///< HTTP 400 Bad Request
            Unauthorized                  = 401,///< HTTP 401 Unauthorized
            PaymentRequired               = 402,///< HTTP 402 Payment Required
            Forbidden                     = 403,///< HTTP 403 Forbidden
            NotFound                      = 404,///< HTTP 404 Not Found
            MethodNotAllowed              = 405,///< HTTP 405 Method Not Allowed
            NotAcceptable                 = 406,///< HTTP 406 Not Acceptable
            ProxyAuthenticationRequired   = 407,///< HTTP 407 Proxy Authentication Required
            RequestTimeout                = 408,///< HTTP 408 Request Timeout
            Conflict                      = 409,///< HTTP 409 Conflict
            Gone                          = 410,///< HTTP 410 Gone
            LengthRequired                = 411,///< HTTP 411 Length Required
            PreconditionFailed            = 412,///< HTTP 412 Precondition Failed
            PayloadTooLarge               = 413,///< HTTP 413 Payload Too Large
            URITooLong                    = 414,///< HTTP 414 URI Too Long
            UnsupportedMediaType          = 415,///< HTTP 415 Unsupported Media Type
            RangeNotSatisfiable           = 416,///< HTTP 416 Range Not Satisfiable
            ExpectationFailed             = 417,///< HTTP 417 Expectation Failed
            ImATeapot                     = 418,///< HTTP 418 I'm a teapot
            MisdirectedRequest            = 421,///< HTTP 421 Misdirected Request
            UnprocessableEntity           = 422,///< HTTP 422 Unprocessable Entity
            Locked                        = 423,///< HTTP 423 Locked
            FailedDependency              = 424,///< HTTP 424 Failed Dependency
            TooEarly                      = 425,///< HTTP 425 Too Early
            UpgradeRequired               = 426,///< HTTP 426 Upgrade Required
            PreconditionRequired          = 428,///< HTTP 428 Precondition Required
            TooManyRequests               = 429,///< HTTP 429 Too Many Requests
            RequestHeaderFieldsTooLarge   = 431,///< HTTP 431 Request Header Fields Too Large
            UnavailableForLegalReasons    = 451,///< HTTP 451 Unavailable For Legal Reasons
            InternalServerError           = 500,///< HTTP 500 Internal Server Error
            NotImplemented                = 501,///< HTTP 501 Not Implemented
            BadGateway                    = 502,///< HTTP 502 Bad Gateway
            ServiceUnavailable            = 503,///< HTTP 503 Service Unavailable
            GatewayTimeout                = 504,///< HTTP 504 Gateway Timeout
            HTTPVersionNotSupported       = 505,///< HTTP 505 HTTP Version Not Supported
            VariantAlsoNegotiates         = 506,///< HTTP 506 Variant Also Negotiates
            InsufficientStorage           = 507,///< HTTP 507 Insufficient Storage
            LoopDetected                  = 508,///< HTTP 508 Loop Detected
            NotExtended                   = 510,///< HTTP 510 Not Extended
            NetworkAuthenticationRequired = 511 ///< HTTP 511 Network Authentication Required
        };

        /// @brief Converts an Status enum to its corresponding string representation.
        /// @param status The Status enum value.
        /// @return A string representing the HTTP status code and message.
        constexpr static auto Status_string(Status status) -> std::string_view {
            switch (status) {
                case Status::Continue:
                    return "100 Continue";
                case Status::SwitchingProtocols:
                    return "101 Switching Protocols";
                case Status::Processing:
                    return "102 Processing";
                case Status::OK:
                    return "200 OK";
                case Status::Created:
                    return "201 Created";
                case Status::Accepted:
                    return "202 Accepted";
                case Status::NonAuthoritativeInformation:
                    return "203 Non-Authoritative Information";
                case Status::NoContent:
                    return "204 No Content";
                case Status::ResetContent:
                    return "205 Reset Content";
                case Status::PartialContent:
                    return "206 Partial Content";
                case Status::MultiStatus:
                    return "207 Multi-Status";
                case Status::AlreadyReported:
                    return "208 Already Reported";
                case Status::IMUsed:
                    return "226 IM Used";
                case Status::MultipleChoices:
                    return "300 Multiple Choices";
                case Status::MovedPermanently:
                    return "301 Moved Permanently";
                case Status::Found:
                    return "302 Found";
                case Status::SeeOther:
                    return "303 See Other";
                case Status::NotModified:
                    return "304 Not Modified";
                case Status::UseProxy:
                    return "305 Use Proxy";
                case Status::TemporaryRedirect:
                    return "307 Temporary Redirect";
                case Status::PermanentRedirect:
                    return "308 Permanent Redirect";
                case Status::BadRequest:
                    return "400 Bad Request";
                case Status::Unauthorized:
                    return "401 Unauthorized";
                case Status::PaymentRequired:
                    return "402 Payment Required";
                case Status::Forbidden:
                    return "403 Forbidden";
                case Status::NotFound:
                    return "404 Not Found";
                case Status::MethodNotAllowed:
                    return "405 Method Not Allowed";
                case Status::NotAcceptable:
                    return "406 Not Acceptable";
                case Status::ProxyAuthenticationRequired:
                    return "407 Proxy Authentication Required";
                case Status::RequestTimeout:
                    return "408 Request Timeout";
                case Status::Conflict:
                    return "409 Conflict";
                case Status::Gone:
                    return "410 Gone";
                case Status::LengthRequired:
                    return "411 Length Required";
                case Status::PreconditionFailed:
                    return "412 Precondition Failed";
                case Status::PayloadTooLarge:
                    return "413 Payload Too Large";
                case Status::URITooLong:
                    return "414 URI Too Long";
                case Status::UnsupportedMediaType:
                    return "415 Unsupported Media Type";
                case Status::RangeNotSatisfiable:
                    return "416 Range Not Satisfiable";
                case Status::ExpectationFailed:
                    return "417 Expectation Failed";
                case Status::ImATeapot:
                    return "418 I'm a teapot";
                case Status::MisdirectedRequest:
                    return "421 Misdirected Request";
                case Status::UnprocessableEntity:
                    return "422 Unprocessable Entity";
                case Status::Locked:
                    return "423 Locked";
                case Status::FailedDependency:
                    return "424 Failed Dependency";
                case Status::TooEarly:
                    return "425 Too Early";
                case Status::UpgradeRequired:
                    return "426 Upgrade Required";
                case Status::PreconditionRequired:
                    return "428 Precondition Required";
                case Status::TooManyRequests:
                    return "429 Too Many Requests";
                case Status::RequestHeaderFieldsTooLarge:
                    return "431 Request Header Fields Too Large";
                case Status::UnavailableForLegalReasons:
                    return "451 Unavailable For Legal Reasons";
                case Status::InternalServerError:
                    return "500 Internal Server Error";
                case Status::NotImplemented:
                    return "501 Not Implemented";
                case Status::BadGateway:
                    return "502 Bad Gateway";
                case Status::ServiceUnavailable:
                    return "503 Service Unavailable";
                case Status::GatewayTimeout:
                    return "504 Gateway Timeout";
                case Status::HTTPVersionNotSupported:
                    return "505 HTTP Version Not Supported";
                case Status::VariantAlsoNegotiates:
                    return "506 Variant Also Negotiates";
                case Status::InsufficientStorage:
                    return "507 Insufficient Storage";
                case Status::LoopDetected:
                    return "508 Loop Detected";
                case Status::NotExtended:
                    return "510 Not Extended";
                case Status::NetworkAuthenticationRequired:
                    return "511 Network Authentication Required";
                default:
                    return "Undefined Status";
            }
        }

        /// @brief Overloads the << operator to output the string representation of an Status.
        /// @param os The output stream.
        /// @param status The Status enum value.
        /// @return The output stream with the HTTP status string.
        std::ostream &operator<<(std::ostream &os, const Status &status) {
            return os << Status_string(status);
        }

    }// namespace http
}// namespace harbour

/// @brief Allow Status to be formatted using fmtlib
template<>
struct fmt::formatter<harbour::http::Status> : formatter<string_view> {
    auto format(harbour::http::Status status, format_context &ctx) const -> format_context::iterator {
        return formatter<string_view>::format(harbour::http::Status_string(status), ctx);
    }
};