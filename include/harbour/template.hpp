///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file template.hpp
/// @brief This file contains the implementation of harbours template rendering

#pragma once

#include <fstream>
#include <string>
#include <iterator>
#include <future>

#include <fmt/core.h>
#include <fmt/format.h>

namespace harbour {
    namespace tmpl {
        namespace detail {

            /// @brief Custom stream buffer to write to a string.
            /// https://stackoverflow.com/a/8737787
            class custombuf : public std::streambuf {
            public:
                /// @brief Constructor that initializes the buffer with a target string.
                /// @param target Reference to the target string.
                custombuf(std::string &target) : target_(target) {
                    this->setp(this->buffer_, this->buffer_ + bufsize - 1);
                }

            private:
                std::string &target_;   ///< Reference to the target string.
                enum { bufsize = 8192 };///< Size of the buffer.
                char buffer_[bufsize];  ///< Buffer array.

                /// @brief Handles buffer overflow by appending to the target string.
                /// @param c Character to be added.
                /// @return The character added or EOF.
                int overflow(int c) {
                    if (!traits_type::eq_int_type(c, traits_type::eof())) {
                        *this->pptr() = traits_type::to_char_type(c);
                        this->pbump(1);
                    }
                    this->target_.append(this->pbase(), this->pptr() - this->pbase());
                    this->setp(this->buffer_, this->buffer_ + bufsize - 1);
                    return traits_type::not_eof(c);
                }

                /// @brief Synchronizes the buffer with the target string.
                /// @return 0 on success.
                int sync() {
                    this->overflow(traits_type::eof());
                    return 0;
                }
            };

        };// namespace detail

        /// @brief Loads the content of a file into a string.
        /// @param path Path to the file.
        /// @return Optional string containing the file content.
        auto load_file(const std::string_view path) -> std::optional<std::string> {
            auto path_str = std::string(path);
            std::ifstream f(path_str);
            if (!f.is_open()) return {};
            std::string s;
            detail::custombuf sbuf(s);
            if (std::ostream(&sbuf)
                << std::ifstream(path_str, std::ios::binary).rdbuf()
                << std::flush) {
                return s;
            }

            return {};
        }

        /// @brief Asynchronously loads the content of a file into a string.
        /// @param path Path to the file.
        /// @return Future containing an optional string with the file content.
        auto load_file_async(const std::string_view path) -> std::future<std::optional<std::string>> {
            return std::async(load_file, std::string(path));
        }

        /// @brief Renders a template string with the provided arguments.
        /// @param tmpl Template string.
        /// @param args Arguments to format the template.
        /// @return Formatted string.
        auto render(const std::string_view tmpl, const auto &...args) -> std::string {
            return fmt::vformat(tmpl, fmt::make_format_args(args...));
        }

        /// @brief Renders a template file with the provided arguments.
        /// @param path Path to the template file.
        /// @param args Arguments to format the template.
        /// @return Optional string containing the formatted content.
        auto render_file(const std::string_view path, const auto &...args) -> std::optional<std::string> {
            if (auto data = load_file(path)) {
                return fmt::vformat(*data, fmt::make_format_args(args...));
            }
            return {};
        }

    }// namespace tmpl
}// namespace harbour