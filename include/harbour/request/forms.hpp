///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file forms.hpp
/// @brief Contains the implementation of harbours http form parser

#pragma once

#include <string>

#include "headers.hpp"

namespace harbour {
    namespace request {
        namespace detail {

            /// @brief A class to parse form data from a URL-encoded string.
            class FormDataParser {
            public:
                /// @brief Represents the current state of the parser.
                enum class State {
                    Key,  ///< Parsing the key part of a key-value pair.
                    Value,///< Parsing the value part of a key-value pair.
                    Done  ///< Parsing is complete.
                };

                /// @brief Constructs a FormDataParser with the given data.
                /// @param data The URL-encoded string to parse.
                [[nodiscard]] explicit FormDataParser(const std::string_view data) : m_data(data), m_state(State::Key), m_pos(0) {}

                /// @brief Parses the form data.
                /// @return A Headers object containing the parsed key-value pairs.
                auto parse() -> request::Headers {
                    request::Headers form_data;
                    std::string_view key, value;

                    while (m_state != State::Done) {
                        switch (m_state) {
                            case State::Key:
                                key     = extract_until('=');
                                m_state = State::Value;
                                break;
                            case State::Value:
                                value          = extract_until('&');
                                form_data[key] = value;
                                m_state        = (m_pos < m_data.size()) ? State::Key : State::Done;
                                break;
                            case State::Done:
                                break;
                        }
                    }

                    return form_data;
                }

            private:
                /// @brief Extracts a substring from the current position until the given delimiter is found.
                /// @param delimiter The character to stop at.
                /// @return The extracted substring.
                auto extract_until(char delimiter) -> std::string_view {
                    auto begin       = &m_data[m_pos];
                    std::size_t size = 0;
                    while (m_pos < m_data.size() && m_data[m_pos] != delimiter) {
                        m_pos++;
                        size++;
                    }
                    m_pos++;// Skip the delimiter
                    return std::string_view(begin, size);
                }

                const std::string_view m_data;///< The URL-encoded string to parse.
                State m_state;                ///< The current state of the parser.
                size_t m_pos;                 ///< The current position in the string.
            };

        }// namespace detail
    }// namespace request
}// namespace harbour