#pragma once
#include <string_view>
#include <harbour/config.hpp>
#include <harbour/harbour.hpp>

#if HARBOUR_ENABLE_COMPRESSION
    #include <brotli/encode.h>
#endif

namespace Http {
    // Log all connections to the server
    auto Logger(Server &ctx, const Request &req, Response &resp) noexcept -> void {
        Logger::info(req.path);
    }

    // Serve an index.html if it exists for any path ending in /
    auto DefaultIndex(Server &ctx, const Request &req, Response &resp) noexcept -> void {
        // if our path ends in / and it isnt a route, serve that dirs index.html
        if (req.path.ends_with("/") && !ctx.get_route(req).has_value()) {
            if (auto index = ctx.cache[req.path + "index.html"]) {
                resp.set_status(Status::Ok);
                resp.set_header("Content-Type", "text/html");
                resp.set_content(*index);
            } else {
                Logger::error("Unable to find index page!");
                resp = Response(Status::InternalServerError);
            }
        }
    }

    // Serve any arbitrary file stored in our cache
    // If the file doesnt exist or its not a supported mime type
    // Return an InternalServerError
    auto FileServer(Server &ctx, const Request &req, Response &resp) noexcept -> void {
        // Get the mime type to use, return empty if its not in our accepted list
        auto get_mime_type = [](const std::string_view ext) -> Result<std::string_view> {
            for (const auto &mimes: ServerAcceptedMimeTypes) {
                const auto &extensions = mimes.first;
                if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end()) {
                    return mimes.second;
                }
            }
            return Err(std::format("Client requested invalid extension: {}", ext));
        };

        // If our path exists and isnt a route serve the requested file
        if (auto content = ctx.cache[req.path]; content.has_value() && !ctx.get_route(req).has_value() && !req.path.ends_with("/")) {
            auto ext = fs::path(req.path).extension().string();
            if (auto mime = get_mime_type(ext)) {
                resp.set_status(Status::Ok);
                resp.set_header("Content-Type", *mime);
                resp.set_content(*content);
            } else {
                // Mime type was not accepted
                Logger::warning(mime.error());
                resp = Response(Status::InternalServerError);
            }
        }
    }

    // Handle file not found
    auto NotFound(Server &ctx, const Request &req, Response &resp) noexcept -> void {
        // If the path doesnt exist and isnt a route, serve our 404 page
        if (!ctx.cache[req.path] && !ctx.cache[req.path + "index.html"] && !ctx.get_route(req).has_value()) {
            if (auto file = ctx.cache["/404.html"]) {
                resp.set_status(Status::NotFound);
                resp.set_header("Content-Type", "text/html");
                resp.set_content(*file);
                Logger::info(std::format("Client got 404: {}", req.path));
            } else {
                Logger::error("Unable to find 404 page!");
                resp = Response(Status::InternalServerError);
            }
        }
    }

#if HARBOUR_ENABLE_COMPRESSION
    // Enable brotli compression
    auto Compression(Server &ctx, const Request &req, Response &resp) -> void {
        if (auto encoding = req.get_header("Accept-Encoding")) {
            if (encoding->find("br") != std::string::npos) {
                if (!resp.content.empty()) {
                    std::string compressed;
                    size_t max_compressed_size = BrotliEncoderMaxCompressedSize(resp.content.size());
                    compressed.resize(max_compressed_size);
                    if (BrotliEncoderCompress(
                                BROTLI_DEFAULT_QUALITY,
                                BROTLI_DEFAULT_WINDOW,
                                BROTLI_MODE_GENERIC,
                                resp.content.size(),
                                reinterpret_cast<const uint8_t *>(resp.content.c_str()),
                                &max_compressed_size,
                                reinterpret_cast<uint8_t *>(&compressed[0]))) {
                        resp.set_header("Content-Encoding", "br");
                        resp.set_content(compressed);
                    } else {
                        // Compression failure
                        Logger::error("Brotli compression failed\n");
                        resp = Response(Status::InternalServerError);
                    }
                }
            }
        }
    }
#endif
}// namespace Http
