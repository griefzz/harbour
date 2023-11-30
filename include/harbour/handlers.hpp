#include <harbour/harbour.hpp>
#include "base64.hpp"

namespace Http {
    struct ServeFile {
        /// @brief Serve a file either from cache or from disk if not stored
        /// @param path Path to serve
        explicit ServeFile(std::filesystem::path path) : path(std::move(path)) {}
        auto operator()(Server &ctx, const Request &req) const noexcept -> Response {
            // Get the mime type to use, return empty if its not in our accepted list
            auto get_mime_type = [](const std::string_view ext) -> Result<std::string_view> {
                for (const auto &mimes: ServerAcceptedMimeTypes()) {
                    const auto &extensions = mimes.first;
                    if (std::find(extensions.begin(), extensions.end(), ext) != extensions.end()) {
                        return mimes.second;
                    }
                }
                return Err(std::format("Client requested invalid extension: {}", ext));
            };

            if (auto cached = ctx.cache[path]) {
                return Html(*cached);
            } else {
                if (auto disk = read_file(path)) {
                    if (auto mime = get_mime_type(path.extension().string())) {
                        auto resp = Response(Status::Ok);
                        resp.set_header("Content-Type", *mime);
                        resp.set_content(*disk);
                        return resp;
                    } else {
                        Logger::error(mime.error());
                        return Status::InternalServerError;
                    }
                } else {
                    Logger::error(FileMapError_string(disk.error()));
                    return Status::InternalServerError;
                }
            }
        }

        std::filesystem::path path;
    };

    struct RequireAuth {
        /// @brief Require authentication to access the route of this handler.
        ///        Key should be of the form username:password
        /// @param key Key to use for authorization
        /// @param handler Handler to apply requirement to
        explicit RequireAuth(const std::string &key, Handler handler) : key(Base64::encode(key)), handler(std::move(handler)) {}
        auto operator()(Server &ctx, const Request &req) const noexcept -> Response {
            if (req.get_header("Authorization") == "Basic " + key) {
                return handler(ctx, req);
            }
            return Status::Unauthorized;
        }

        std::string key;
        Handler handler;
    };

    struct RequireMethod {
        /// @brief Require a specific Method to use the Handler
        /// @param m Method to require
        /// @param handler Handler to apply requirement to
        explicit RequireMethod(Method m, Handler handler) : m(m), handler(std::move(handler)) {}
        auto operator()(Server &ctx, const Request &req) const noexcept -> Response {
            if (req.method != m) {
                return handler(ctx, req);
            }
            return Status::NotImplemented;
        }

        Method m;
        Handler handler;
    };
}// namespace Http
