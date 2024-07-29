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

            std::ifstream file(std::string(path), std::ios::binary);
            if (file.is_open())
                std::string((It(file)), It());

            return {};
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

    }// namespace tmpl
}// namespace harbour