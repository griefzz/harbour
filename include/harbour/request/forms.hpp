///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file forms.hpp
/// @brief Contains the implementation of harbours http form data parser

#pragma once

#include <string>

#include "headers.hpp"

namespace harbour {
    namespace request {
        namespace detail {
            namespace FormData {

                /// @brief Parse form data from a HTTP Request
                /// @param data Form data string for example: 'name=bob&id=123'
                /// @return request::Headers Parsed form data as a map
                static auto parse(const std::string_view data) -> request::Headers {
                    request::Headers form;
                    size_t start = 0;
                    size_t end   = 0;

                    while (end != std::string::npos) {
                        end          = data.find('&', start);
                        size_t equal = data.find('=', start);

                        if (equal != std::string::npos && (end == std::string::npos || equal < end)) {
                            auto key   = data.substr(start, equal - start);
                            auto value = data.substr(equal + 1, end - equal - 1);
                            form[key]  = value;
                        }

                        start = end + 1;
                    }

                    return form;
                }

            }// namespace FormData
        }// namespace detail
    }// namespace request
}// namespace harbour