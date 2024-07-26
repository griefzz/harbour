///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///

#pragma once

#include <filesystem>
#include <vector>
#include <utility>
#include <string>
#include <optional>

#include <harbour/harbour.hpp>

#include "templates.hpp"

using namespace harbour;
namespace fs = std::filesystem;

/// @brief Accepted MIME Types
using MimeTypes = std::vector<std::pair<std::vector<std::string>, std::string>>;
static auto ServerAcceptedMimeTypes() -> const MimeTypes & {
    static const MimeTypes mimes = {{{".txt", ".text", ".conf", ".log", ".ini"}, "text/plain; charset=utf-8"},
                                    {{".html", ".htm"}, "text/html; charset=utf-8"},
                                    {{".pdf"}, "application/pdf"},
                                    {{".jpg", ".jpeg"}, "image/jpeg"},
                                    {{".png"}, "image/png"},
                                    {{".gif"}, "image/gif"},
                                    {{".mp3"}, "audio/mpeg"},
                                    {{".mp4"}, "video/mp4"},
                                    {{".json"}, "application/json"},
                                    {{".xml"}, "application/xml"},
                                    {{".zip"}, "application/zip"},
                                    {{".css"}, "text/css"},
                                    {{".doc", ".docx"}, "application/msword"},
                                    {{".xls", ".xlsx"}, "application/vnd.ms-excel"},
                                    {{".ppt", ".pptx"}, "application/vnd.ms-powerpoint"},
                                    {{".wav"}, "audio/wav"},
                                    {{".avi"}, "video/x-msvideo"},
                                    {{".bmp"}, "image/bmp"},
                                    {{".csv"}, "text/csv"},
                                    {{".js"}, "application/javascript"},
                                    {{".ico"}, "image/x-icon"},
                                    {{".svg", ".svgz"}, "image/svg+xml"},
                                    {{".c", ".cpp", ".h", ".hpp"}, "text/html; charset=utf-8"},
                                    {{".tar"}, "application/x-tar"},
                                    {{".7z"}, "application/x-7z-compressed"},
                                    {{".rar"}, "application/x-rar-compressed"},
                                    {{".rtf"}, "application/rtf"},
                                    {{".ogg"}, "audio/ogg"},
                                    {{".ogv"}, "video/ogg"},
                                    {{".webm"}, "video/webm"},
                                    {{".flv"}, "video/x-flv"},
                                    {{".mkv"}, "video/x-matroska"},
                                    {{".eot"}, "application/vnd.ms-fontobject"},
                                    {{".ttf"}, "font/ttf"},
                                    {{".woff"}, "font/woff"},
                                    {{".woff2"}, "font/woff2"},
                                    {{".otf"}, "font/otf"},
                                    {{".md"}, "text/markdown"}};

    return mimes;
}

auto get_mime_type(const std::string_view ext) -> std::string {
    for (const auto &mimes: ServerAcceptedMimeTypes()) {
        const auto &extensions = mimes.first;
        if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end()) {
            return mimes.second;
        }
    }

    log::warn("Invalid mime type for: {}", ext);
    return "text/plain";
};

auto get_full_path(auto &&target) -> std::string {
    auto current_dir = fs::current_path();
    current_dir += target;
    return current_dir.string();
}

auto get_directory_contents(auto &&path) -> std::optional<std::vector<std::filesystem::path>> {
    if (std::filesystem::exists(path)) {
        std::vector<std::filesystem::path> items;
        for (const auto &entry: std::filesystem::directory_iterator(path)) {
            if (entry.is_directory())
                items.push_back(entry.path().string() + std::filesystem::path("/").make_preferred().string());
            else
                items.push_back(entry.path());
        }
        return items;
    } else {
        return {};
    }
}

auto Ship(const Request &req) -> awaitable<Response> {
    // Attempt to serve index.html for paths that end in "/" or ""
    if (req.path.empty() || req.path.back() == '/') {
        fs::path index_path  = req.path.empty() ? std::string(req.path) + "/index.html" : std::string(req.path) + "index.html";
        const auto full_path = get_full_path(fs::path(index_path).make_preferred());
        if (auto file = tmpl::load_file(full_path)) {
            const auto mime = get_mime_type(fs::path(full_path).extension().string());
            co_return Response()
                    .with_status(http::Status::OK)
                    .with_header("Content-Type", mime)
                    .with_data(*file);
        } else {
            // If no index.html file exists create our own
            auto dir_path = std::filesystem::current_path().string() + std::filesystem::path(req.path).make_preferred().string();
            auto contents = get_directory_contents(std::filesystem::current_path().string() + std::filesystem::path(req.path).make_preferred().string());

            // Redirect to / if file/folder not found
            if (!contents.has_value()) {
                log::warn("File not found: {}", dir_path);
                co_return Response().with_redirect("/");
            }

            std::string entries;
            // Add our path traversal directories
            entries += tmpl::render(tmpls::file(), ".", ".");
            entries += tmpl::render(tmpls::file(), "..", "..");
            // Append our directory listings
            for (const auto &content: *contents)
                if (std::filesystem::is_directory(content)) {
                    auto dir = std::filesystem::proximate(content, content.parent_path().parent_path()).string() + "/";
                    entries += tmpl::render(tmpls::file(), dir, dir);
                } else {
                    entries += tmpl::render(tmpls::file(), content.filename().string(), content.filename().string());
                }

            co_return Response()
                    .with_status(http::Status::OK)
                    .with_header("Content-Type", "text/html")
                    .with_data(tmpl::render(tmpls::index(), req.path, entries));
        }

        log::warn("Couldnt find index: {}", full_path);
        co_return http::Status::InternalServerError;
    }

    // If our path exists serve the file
    const auto full_path = get_full_path(fs::path(req.path).make_preferred());
    if (auto file = tmpl::load_file(full_path)) {
        const auto mime = get_mime_type(fs::path(req.path).extension().string());
        co_return Response()
                .with_status(http::Status::OK)
                .with_header("Content-Type", mime)
                .with_data(*file);
    }

    log::warn("File not found: {}", req.path);
    co_return http::Status::NotFound;
}