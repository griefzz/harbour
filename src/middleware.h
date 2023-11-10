#pragma once
#include <optional>
#include <string_view>
#include "config.h"
#include "cache.h"
#include "server.h"
#include "logger.h"
#include "config.h"

namespace Middleware {
    // Log all connections to the server
    auto Logger(Server &ctx, const Request &req, Response &resp) -> void {
        Logger::info(req.path);
    }

    // Serve an index.html fif it exists for any path ending in /
    auto DefaultIndex(Server &ctx, const Request &req, Response &resp) -> void {
        if (ctx.is_route(req)) {
            return;
        }

        if (req.path.ends_with("/")) {
            if (auto index = ctx.cache[req.path + "index.html"]) {
                resp.set_type(ResponseType::Ok);
                resp.set_header("Content-Type", "text/html");
                resp.set_content(*index);
            } else {
                Logger::error("Unable to find index page!");
                resp = Response(ResponseType::InternalServerError);
            }
        }
    }

    // Serve any arbitrary file stored in our cache
    // If the file doesnt exist or its not a supported mime type
    // Return an InternalServerError
    auto FileServer(Server &ctx, const Request &req, Response &resp) -> void {
        if (ctx.is_route(req)) {
            return;
        }

        // Get the mime type to use, return empty if its not in our accepted list
        auto get_mime_type = [](const std::string_view ext) -> std::optional<std::string_view> {
            for (const auto &mimes: ServerAcceptedMimeTypes) {
                const auto &extensions = mimes.first;
                if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end()) {
                    return mimes.second;
                }
            }
            return {};
        };

        if (auto content = ctx.cache[req.path]) {
            auto ext = fs::path(req.path).extension().string();
            if (auto mime = get_mime_type(ext)) {
                resp.set_type(ResponseType::Ok);
                resp.set_header("Content-Type", *mime);
                resp.set_content(*content);
            } else {
                // Mime type was not accepted
                Logger::warning(std::format("Client requested invalid extension: {}", ext));
                resp = Response(ResponseType::InternalServerError);
            }
        }
    }

    // Handle file not found
    auto NotFound(Server &ctx, const Request &req, Response &resp) -> void {
        if (ctx.is_route(req)) {
            return;
        }

        if (auto content = ctx.cache[req.path]; !content.has_value()) {
            if (auto file = ctx.cache["/404.html"]) {
                resp.set_type(ResponseType::NotFound);
                resp.set_header("Content-Type", "text/html");
                resp.set_content(*file);
                Logger::info(std::format("Client got 404: {}", req.path));
            } else {
                Logger::error("Unable to find 404 page!");
                resp = Response(ResponseType::InternalServerError);
            }
        }
    }
}// namespace Middleware
