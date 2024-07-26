///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file method.hpp
/// @brief Contains the implementation of harbours http method type

#pragma once

namespace harbour {
    namespace http {

        enum class Method {
            GET, ///< GET method
            POST,///< POST method
            PUT, ///< PUT method (todo)
            HEAD ///< HEAD method (todo)
        };

    }// namespace http
}// namespace harbour