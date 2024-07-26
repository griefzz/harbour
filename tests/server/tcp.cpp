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
    auto ship_handler = [&](const Request &req, Response &resp) -> awaitable<void> {
        resp = Echo(req);
        co_return;
    };
    auto settings = server::Settings::defaults();
    return server::Server(ship_handler, settings, ships);
}

auto client(asio::io_context &io_context, const server::Settings &settings, bool &ok) -> awaitable<void> {
    try {
        tcp::socket socket(io_context);
        tcp::endpoint endpoint(asio::ip::make_address("127.0.0.1"), settings.port);

        co_await socket.async_connect(endpoint, use_awaitable);
        co_await asio::async_write(socket, asio::buffer(req), use_awaitable);

        char data[4096];
        auto n   = co_await socket.async_read_some(asio::buffer(data, 4096), use_awaitable);
        auto got = std::string(data, n);
        if (got == want)
            ok = true;
    } catch (const std::exception &e) {
        log::critical("client exception: {}", e.what());
    }

    io_context.stop();
}

auto main() -> int {
    try {
        asio::io_context io_context(1);

        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { io_context.stop(); });

        auto srv = make_server();
        co_spawn(io_context, srv.listener(), asio::detached);

        bool ok = false;
        co_spawn(io_context, client(io_context, srv.settings, ok), asio::detached);

        io_context.run();

        assert(ok);
        if (!ok) return 1;

        return 0;
    } catch (const std::exception &e) {
        log::critical("Server exception: {}", e.what());
        return 1;
    }
}