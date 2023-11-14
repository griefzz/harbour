#include "server.hpp"
#include "base64.hpp"

namespace Handlers {
    // Serve a file either from cache or from disk if not stored
    struct ServeFile {
        explicit ServeFile(std::filesystem::path path) : path(path) {}
        auto operator()(Server &ctx, const Request &req, Response &resp) noexcept -> void {
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

            if (auto cached = ctx.cache[path]) {
                resp.set_type(ResponseType::Ok);
                resp.set_header("Content-Type", "text/html");
                resp.set_content(*cached);
            } else {
                if (auto disk = read_file(path)) {
                    if (auto mime = get_mime_type(path.extension().string())) {
                        resp.set_type(ResponseType::Ok);
                        resp.set_header("Content-Type", *mime);
                        resp.set_content(*disk);
                    } else {
                        Logger::error(mime.error());
                        resp = Response(ResponseType::InternalServerError);
                    }

                } else {
                    Logger::error(fme_to_string(disk.error()));
                    resp = Response(ResponseType::InternalServerError);
                }
            }
        }

        std::filesystem::path path;
    };

    // Require authentication to access the route of this handler
    // key should be of the form username:password
    struct RequireAuth {
        explicit RequireAuth(const std::string &key, Handler handler) : key(base64::encode(key)), handler(handler) {}
        auto operator()(Server &ctx, const Request &req, Response &resp) noexcept -> void {
            if (req["Authorization"] == "Basic " + key) {
                handler(ctx, req, resp);
            } else {
                resp = Response(ResponseType::Unauthorized);
            }
        }

        std::string key;
        Handler handler;
    };
}// namespace Handlers
