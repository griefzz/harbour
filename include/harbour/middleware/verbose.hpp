///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file verbose.hpp
/// @brief Contains the implementation of harbours verbose middleware
#pragma once

#include "../request/request.hpp"
#include "../log/log.hpp"

namespace harbour {
    namespace middleware {

        /// @brief Verbose logging middleware.
        ///        Will print the ip:port -> req.path for a connection.
        /// @param req Request to use
        auto Verbose(const Request &req) -> void {
            log::info("{}:{} → {}", req.socket->address(), req.socket->port(), req.path);
        }

    }// namespace middleware
}// namespace harbour