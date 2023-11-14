#pragma once
#include <chrono>
#include <format>
#include <iostream>
#include <source_location>
#include <string>

namespace Logger {
    auto current_time() noexcept -> std::string {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return buffer;
    }

    auto info(const std::string_view message) noexcept -> void {
        std::clog << std::format("{} {}: {}\n", "INFO", current_time(), message);
    }

    auto warning(const std::string_view message) noexcept -> void {
        std::clog << std::format("{} {}: {}\n", "WARNING", current_time(), message);
    }

    auto error(const std::string_view message, const std::source_location location = std::source_location::current()) noexcept -> void {
        std::clog << std::format("{} {}: {}({}:{}) `{}`: {}\n", "ERROR", current_time(),
                                 location.file_name(), location.line(), location.column(),
                                 location.function_name(), message);
    }
};// namespace Logger