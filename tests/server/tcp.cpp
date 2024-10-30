#include <cassert>
#include <vector>
#include <string>

#include <asio.hpp>
#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/ip/address.hpp>
#include <asio/ssl.hpp>

#include <harbour/harbour.hpp>

using namespace harbour;
using namespace asio::ip;

const std::string req  = "GET / HTTP/1.1\r\n\r\n";
const std::string want = "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\nConnection: keep-alive\nContent-Length: 18\n\nGET / HTTP/1.1\r\n\r\n";

auto Echo(const Request &req) -> Response {
    return req.data;
}

auto make_server() {
    std::vector<harbour::detail::Ship> ships;
    auto ship_handler = [&](const Request &req, Response &resp) -> asio::awaitable<void> {
        resp = Echo(req);
        co_return;
    };
    auto settings = server::Settings::defaults();
    return server::Server(ship_handler, settings, ships);
}

auto client(asio::io_context &io_context, const server::Settings &settings) -> asio::awaitable<bool> {
    try {
        auto executor = co_await asio::this_coro::executor;
        tcp::socket socket(executor);
        
        tcp::endpoint endpoint(asio::ip::make_address("127.0.0.1"), settings.port);
        co_await socket.async_connect(endpoint, asio::use_awaitable);
        
        co_await async_write(socket, asio::buffer(req), asio::use_awaitable);

        std::array<char, 4096> data;
        auto n = co_await socket.async_read_some(asio::buffer(data), asio::use_awaitable);
        auto got = std::string_view(data.data(), n);
        
        co_return got == want;
    } catch (const std::exception &e) {
        log::critical("client exception: {}", e.what());
        co_return false;
    }
}

auto main() -> int {
    try {
        asio::io_context io_context(1);
        auto guard = asio::make_work_guard(io_context);

        // Signal handling
        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { 
            guard.reset();
            io_context.stop();
        });

        // Create and start server
        auto srv = make_server();
        
        // Use structured concurrency pattern
        bool ok = false;
        asio::co_spawn(io_context, 
            [&]() -> asio::awaitable<void> {
                asio::co_spawn(
                    co_await asio::this_coro::executor,
                    srv.listener(),
                    asio::detached
                );
                
                // Run client and get result
                ok = co_await client(io_context, srv.settings_);
                io_context.stop();
            }, 
            asio::detached
        );

        io_context.run();
        
        assert(ok);
        return ok ? 0 : 1;

    } catch (const std::exception& e) {
        log::critical("Server exception: {}", e.what());
        return 1;
    }
}