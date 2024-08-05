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
#include <filesystem>

#include <asio.hpp>

#include <fmt/core.h>
#include <fmt/format.h>

namespace harbour {
    namespace tmpl {

        /// @brief Loads the content of a file into a string.
        /// @param path Path to the file.
        /// @return Optional string containing the file content.
        auto load_file(const std::string_view path) -> std::optional<std::string> {
            using It = std::istreambuf_iterator<char>;

            if (!std::filesystem::exists(path))
                return {};

            std::ifstream file(path.data(), std::ios::binary);
            if (file.is_open())
                return std::string((It(file)), It());

            return {};
        }

        namespace detail {

            /// @brief load file with callback used in load_file_async
            template<typename Callback>
            void load_file_impl(const std::string_view path, Callback cb) {
                std::thread(
                        [path, cb = std::move(cb)]() mutable {
                            auto data = load_file(path);
                            std::move(cb)(std::move(data));
                        })
                        .detach();
            }

        }// namespace detail

        /// @brief Loads the content of a file into a string using a completion token
        /// @tparam CompletionToken Completion Token for async operation
        /// @param path Path to the file.
        /// @param token Token to specify async operation type (asio::use_future, asio::use_awaitable)
        /// @return Optional string containing the file content.
        template<asio::completion_token_for<void(std::optional<std::string>)> CompletionToken>
        auto load_file_async(const std::string_view path, CompletionToken &&token) {
            auto init = [](asio::completion_handler_for<void(std::optional<std::string>)> auto handler,
                           const std::string_view path) {
                auto work = asio::make_work_guard(handler);

                detail::load_file_impl(path,
                                       [handler = std::move(handler),
                                        work    = std::move(work)](std::optional<std::string> result) mutable {
                                           auto alloc = asio::get_associated_allocator(
                                                   handler, asio::recycling_allocator<void>());

                                           asio::dispatch(work.get_executor(),
                                                          asio::bind_allocator(alloc,
                                                                               [handler = std::move(handler),
                                                                                result  = std::optional<std::string>(result)]() mutable {
                                                                                   std::move(handler)(result);
                                                                               }));
                                       });
            };

            return asio::async_initiate<CompletionToken, void(std::optional<std::string>)>(
                    init,
                    token,
                    path);
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
            if (auto data = load_file(path))
                return fmt::vformat(*data, fmt::make_format_args(args...));

            return {};
        }

        namespace detail {
            /// @brief render a file with callback used in render_file_async
            template<typename Callback>
            void render_file_impl(Callback cb, const std::string_view path, const auto &...args) {
                std::thread(
                        [path, cb = std::move(cb), args...]() mutable {
                            auto data = render_file(path, args...);
                            std::move(cb)(std::move(data));
                        })
                        .detach();
            }

        }// namespace detail

        template<asio::completion_token_for<void(std::optional<std::string>)> CompletionToken>
        auto render_file_async(CompletionToken &&token, const std::string_view path, const auto &...args) {
            auto init = [](asio::completion_handler_for<void(std::optional<std::string>)> auto handler,
                           const std::string_view path, const auto &...args) {
                auto work = asio::make_work_guard(handler);

                detail::render_file_impl([handler = std::move(handler), work = std::move(work)](std::optional<std::string> result) mutable {
                                           auto alloc = asio::get_associated_allocator(
                                                   handler, asio::recycling_allocator<void>());

                                           asio::dispatch(work.get_executor(),
                                                          asio::bind_allocator(alloc,
                                                                               [handler = std::move(handler),
                                                                                result  = std::optional<std::string>(result)]() mutable {
                                                                                   std::move(handler)(result);
                                                                               })); }, path, (args)...);
            };

            return asio::async_initiate<CompletionToken, void(std::optional<std::string>)>(
                    init,
                    token,
                    path,
                    (args)...);
        }

    }// namespace tmpl
}// namespace harbour