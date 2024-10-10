///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file files.hpp
/// @brief Contains the implementation of harbours File Server Middleware
#pragma once

#include <filesystem>
#include <optional>
#include <string_view>
#include <ranges>
#include <vector>
#include <utility>

#include <asio/awaitable.hpp>

#include "../request/request.hpp"
#include "../response/response.hpp"
#include "../template.hpp"

namespace harbour::middleware {

    class FileServer {
    public:
        explicit FileServer() = default;
        explicit FileServer(const std::string &working_directory) : working_directory(working_directory) {}

        /// @brief Accepted MIME Types
        using MimeTypes = std::vector<std::pair<std::vector<std::string_view>, std::string_view>>;
        static auto AcceptedMimeTypes() -> const MimeTypes & {
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

        auto get_mime_type(const std::string_view ext) const -> std::string_view {
            for (const auto &mimes: AcceptedMimeTypes()) {
                const auto &extensions = mimes.first;
                if (std::ranges::find(extensions, ext) != extensions.end()) {
                    return mimes.second;
                }
            }

            log::warn("Invalid mime type for: {}", ext);
            return "text/plain";
        };

        /// @brief Serve a file to the user if it exists, otherwise return nothing
        /// @param req Request to use for parsing out the file path
        /// @return Response containing file data on success, nothing on an error
        auto operator()(const Request &req, Response &resp) -> asio::awaitable<std::optional<Response>> {
            // remove the '/' prefix from a string if it exists
            auto trim = [](const auto &str) -> std::string_view {
                if (str.starts_with('/'))
                    return std::string_view(str.begin() + 1, str.end());
                return str;
            };

            // full path to requested file
            auto path = working_directory / trim(req.path);

            // If the path is a directory, assume we want an index.html
            if (path.string().ends_with("/"))
                path = path / "index.html";

            if (std::filesystem::exists(path)) {
                if (auto file = co_await tmpl::load_file_async(path.string(), asio::use_awaitable)) {
                    resp.data            = *file;
                    resp["Content-Type"] = get_mime_type(path.extension().string());
                    co_return std::nullopt;
                }
            }


            // path was not found so return 404
            co_return http::Status::NotFound;
        }

    private:
        std::filesystem::path working_directory{std::filesystem::current_path()};
    };

}// namespace harbour::middleware