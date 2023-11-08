#pragma once
#include <chrono>
#include <format>
#include <iostream>
#include <source_location>
#include <string>

#define LOGGER_FORMAT(type, time, message, loc)                \
    std::format("{} {}: {}({}:{}) `{}`: {}\n", (type), (time), \
                loc.file_name(), loc.line(), loc.column(),     \
                loc.function_name(), message);

namespace Logger {
    auto current_time() -> std::string {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return buffer;
    }

    auto info(const std::string_view message, const std::source_location location = std::source_location::current()) -> void {
        std::clog << LOGGER_FORMAT("INFO", current_time(), message, location);
    }

    auto warning(const std::string_view message, const std::source_location location = std::source_location::current()) -> void {
        std::clog << LOGGER_FORMAT("WARNING", current_time(), message, location);
    }

    auto error(const std::string_view message, const std::source_location location = std::source_location::current()) -> void {
        std::clog << LOGGER_FORMAT("ERROR", current_time(), message, location);
    }
};// namespace Logger