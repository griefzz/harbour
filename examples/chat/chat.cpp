///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file chat.cpp
/// @brief Contains the example implementation of a simple chat server

#include <harbour/harbour.hpp>
#include <string>
#include <unordered_map>
#include <ranges>
#include <random>

using namespace harbour;

struct Chat {
    struct ReadMessage {
        std::string text;
    };

    struct WriteMessage {
        std::string sender;
        std::string text;
    };

    std::unordered_map<std::string, websocket::Connection *> clients;

    // Create a random 4 character username
    auto create_username() -> std::string {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 25);
        std::string username;
        for (int i = 0; i < 4; i++)
            username += static_cast<char>('A' + dis(gen));
        return username;
    }

    // Parse a message from a sender into a message to all clients
    auto create_message(const std::string &sender, const std::string &text) -> std::optional<std::string> {
        try {
            auto in  = deserialize<ReadMessage>(text);
            auto out = WriteMessage{sender, in.text};
            return serialize(out);
        } catch (const std::exception &e) {
            log::warn("Failed to deserialize message: {}", e.what());
            return std::nullopt;
        }
    }

    auto operator()(const Request &req) -> awaitable<std::optional<Response>> try {
        // Upgrade to a websocket connection
        if (auto ws = co_await websocket::upgrade(req)) {
            log::info("WebSocket connection open");
            clients[ws->secret_] = &(*ws);
            auto username        = create_username();

            for (;;) {
                // Read message
                auto msg = co_await ws->read();

                // Connection closed normally
                if (!msg) {
                    // Remove client from map
                    clients.erase(ws->secret_);
                    break;
                }

                log::info("{}: {}", username, *msg);

                // Attempt to parse message from sender
                if (auto message = create_message(username, *msg)) {
                    // Echo message back to all clients
                    for (auto &client: clients | std::views::values) {
                        co_await client->send(*message);
                    }
                }
            }

            log::info("WebSocket connection closed");
            co_return std::nullopt;
        }

        co_return http::Status::BadRequest;
    } catch (const asio::system_error &e) {
        log::warn("WebSocket error: {}", e.what());
        co_return http::Status::BadRequest;
    }
};

// Serve static files from html directory
auto Home() {
    return tmpl::load_file(R"(C:\Users\stone\Desktop\harbour\examples\chat\html\index.html)")
            .value_or("Fuck");
}

auto main() -> int {
    Harbour hb;

    // Serve index.html at root
    hb.dock("/", Home);

    // Handle WebSocket connections at /ws
    hb.dock("/ws", Chat{});

    hb.sail();
    return 0;
}
