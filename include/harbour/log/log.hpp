///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file log.hpp
/// @brief Contains the implementation of harbours logger

#pragma once

#include <array>
#include <string>
#include <functional>
#include <memory>

#include <asio/awaitable.hpp>

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/color.h>
#include <fmt/ranges.h>

namespace harbour {
    namespace log {
        namespace detail {

            // Internal wrapper for printing with a style
            inline void vlog(fmt::text_style style, fmt::string_view fmt, fmt::format_args args) {
                fmt::print(style, "{}", fmt::vformat(fmt, args));
            }

            auto emphasis = fmt::emphasis::bold;

        }// namespace detail

        /// @brief Report an info log to stdout
        /// @param fmt Format for the log
        /// @param ...args Arguments to log
        template<class... T>
        auto info(fmt::format_string<T...> fmt, T &&...args) {
            const auto style = detail::emphasis | fg(fmt::color::gray);
            fmt::print(style, "• ");
            detail::vlog(style, fmt, fmt::make_format_args(args...));
            fmt::print("\n");
        }

        /// @brief Report an info log to stdout
        /// @param arg Argument to log
        auto info(const std::string_view arg) {
            const auto style = detail::emphasis | fg(fmt::color::gray);
            fmt::print(style, "• ");
            fmt::print(style, fmt::runtime(arg));
            fmt::print("\n");
        }

        /// @brief Report a warning log to stdout
        /// @param fmt Format for the log
        /// @param ...args Arguments to log
        template<class... T>
        auto warn(fmt::format_string<T...> fmt, T &&...args) {
            const auto style = detail::emphasis | fg(fmt::color::orange);
            fmt::print(style, "• ");
            detail::vlog(style, fmt, fmt::make_format_args(args...));
            fmt::print("\n");
        }

        /// @brief Report a warning log to stdout
        /// @param arg Argument to log
        auto warn(const std::string_view arg) {
            const auto style = detail::emphasis | fg(fmt::color::orange);
            fmt::print(style, "• ");
            fmt::print(style, fmt::runtime(arg));
            fmt::print("\n");
        }

        /// @brief Report a critical log to stdout
        /// @param fmt Format for the log
        /// @param ...args Arguments to log
        template<class... T>
        auto critical(fmt::format_string<T...> fmt, T &&...args) {
            const auto style = detail::emphasis | fg(fmt::color::red);
            fmt::print(style, "• ");
            detail::vlog(style, fmt, fmt::make_format_args(args...));
            fmt::print("\n");
        }

        /// @brief Report a critical log to stdout
        /// @param arg Argument to log
        auto critical(const std::string_view arg) {
            const auto style = detail::emphasis | fg(fmt::color::red);
            fmt::print(style, "• ");
            fmt::print(style, fmt::runtime(arg));
            fmt::print("\n");
        }

    }// namespace log
}// namespace harbour