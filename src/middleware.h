#pragma once
#include <optional>
#include <string_view>
#include "config.h"
#include "cache.h"
#include "server.h"
#include "logger.h"

// Serve any arbitrary file stored in our cache
// If the file doesnt exist or its not a supported mime type
// Return an InternalServerError
auto file_server(const Request &req, Response &resp) -> void {
    // Get the mime type to use, return empty if its not in our accepted list
    auto get_mime_type = [&](const std::string_view ext) -> std::optional<std::string_view> {
        for (const auto &mimes: ServerAcceptedMimeTypes) {
            // get list of extensions for each mime-type
            const auto &extensions = mimes.first;
            if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end()) {
                return mimes.second;// Extension exists
            }
        }
        return {};// Extension not found
    };

    // is our request a route?
    if (auto s = server.routes.find(req.path); s != server.routes.end()) {
        return;
    }

    // this needs to be a little better with relative pathing, doesnt work recusively yet
    auto path = fs::path(req.path).filename();
    if (auto content = cache[path]; content.has_value()) {
        auto ext = fs::path(req.path).extension().string();
        if (auto mime = get_mime_type(ext)) {
            resp.set_type(ResponseType::Ok);
            resp.set_header("Content-Type", *mime);
            resp.set_content(content.value());
        } else {
            // Mime type was not accepted
            Logger::warning(std::format("Client requested invalid extension: {}", ext));
            resp = Response(ResponseType::InternalServerError);
        }
    }
}

// Handle file not found
auto not_found(const Request &req, Response &resp) -> void {
    // is our request a route?
    if (auto s = server.routes.find(req.path); s != server.routes.end()) {
        return;
    }

    auto path = fs::path(req.path).filename();
    if (auto content = cache[path]; !content.has_value()) {
        if (auto file = cache["404.html"]; file.has_value()) {
            resp.set_type(ResponseType::NotFound);
            resp.set_header("Content-Type", "text/html");
            resp.set_content(file.value());
            Logger::info(std::format("Client got 404: {}", req.path));
        } else {
            Logger::error("Unable to find 404 page!");
            resp = Response(ResponseType::InternalServerError);
        }
    }
}