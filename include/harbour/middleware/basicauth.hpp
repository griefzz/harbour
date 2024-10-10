///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file auth.hpp
/// @brief Contains the implementation of harbours HTTP Basic Authentication middleware
#pragma once

#include <string>

#include "../request/request.hpp"
#include "../response/response.hpp"
#include "../crypto/base64.hpp"

namespace harbour::middleware {

    /// @brief Middleware for doing HTTP Basic Authentication.
    ///        This is not to be used in cases where you need strong security.
    class BasicAuth {
        std::string want;///< Base64 encoded credentials to require

    public:
        /// @brief Construct basic authentication from a username and password
        /// @param username Username for auth
        /// @param password Password for auth
        explicit BasicAuth(const std::string &username, const std::string &password) {
            if (auto hash = harbour::crypto::base64::encode(username + ":" + password)) {
                want = "Basic " + *hash;
            } else {
                throw "Unable to base64 encode username and password in BasicAuth";
            }
        }

        /// @brief Construct basic authentication from formatted credentials
        /// @param credentials Credentials to use in username:password format
        explicit BasicAuth(const std::string &credentials) {
            if (auto hash = harbour::crypto::base64::encode(credentials)) {
                want = "Basic " + *hash;
            } else {
                throw "Unable to base64 encode credentials in BasicAuth";
            }
        }

        /// @brief Restrict access to a route using a specified username and password
        /// @param req Request to use for parsing the Authorization header
        /// @return Do nothing if authentication succeeds, otherwise return a 401 Unauthorized
        auto operator()(const Request &req) -> std::optional<Response> {
            if (auto got = req.header("Authorization"); got && *got == want)
                return std::nullopt;

            return Response()
                    .with_status(http::Status::Unauthorized)
                    .with_header("WWW-Authenticate", "Basic realm=\"staging server\"");
        }
    };

}// namespace harbour::middleware