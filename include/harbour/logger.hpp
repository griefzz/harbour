/// @file logger.hpp
/// @brief Header file for the Logger namespace, which provides logging utilities.
#pragma once

#include <chrono>
#include <format>
#include <iostream>
#include <source_location>
#include <string>
#include <array>

/// @namespace Logger
/// @brief Provides simple logging functions with timestamp and severity level.
namespace Logger {
    /// @brief Gets the current system time as a formatted string.
    /// @return A string representing the current system time in the format YYYY-MM-DD HH:MM:SS.
    static auto current_time() noexcept -> std::string {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::array<char, 80> buffer;
        std::strftime(buffer.data(), buffer.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return buffer.data();
    }

    /// @brief Logs an informational message.
    /// @param message The message to log.
    static auto info(const std::string_view message) noexcept -> void {
        std::clog << std::format("{} {}: {}\n", "INFO", current_time(), message);
    }

    /// @brief Logs a warning message.
    /// @param message The message to log.
    static auto warning(const std::string_view message) noexcept -> void {
        std::clog << std::format("{} {}: {}\n", "WARNING", current_time(), message);
    }

    /// @brief Logs an error message with source location information.
    /// @param message The message to log.
    /// @param location The source location from where the error is logged, defaults to the current location.
    static auto error(const std::string_view message, const std::source_location location = std::source_location::current()) noexcept -> void {
        std::clog << std::format("{} {}: {}({}:{}) `{}`: {}\n", "ERROR", current_time(),
                                 location.file_name(), location.line(), location.column(),
                                 location.function_name(), message);
    }
};// namespace Logger