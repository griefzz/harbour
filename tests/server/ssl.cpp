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

// Test certificate and key in PEM format
const std::string TEST_CERT = R"(-----BEGIN CERTIFICATE-----
MIID4zCCAsugAwIBAgIUa2bsx2fiVs/ATUCVb/hsp+zzSwIwDQYJKoZIhvcNAQEL
BQAwgYAxCzAJBgNVBAYTAlVTMQswCQYDVQQIDAJDQTERMA8GA1UEBwwIU2hpcHlh
cmQxFDASBgNVBAoMC0hhcmJvdXIgQ28uMREwDwYDVQQLDAhkZWNraGFuZDEoMCYG
CSqGSIb3DQEJARYZc3BlbmNlci53LnN0b25lQGdtYWlsLmNvbTAeFw0yNDA2MDUw
MjA0MDRaFw0yNTA2MDUwMjA0MDRaMIGAMQswCQYDVQQGEwJVUzELMAkGA1UECAwC
Q0ExETAPBgNVBAcMCFNoaXB5YXJkMRQwEgYDVQQKDAtIYXJib3VyIENvLjERMA8G
A1UECwwIZGVja2hhbmQxKDAmBgkqhkiG9w0BCQEWGXNwZW5jZXIudy5zdG9uZUBn
bWFpbC5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCS5jRGQqKi
Ac2+aujrOzGhGL8p2QeC9BFjlcxwkwp4/mbePamcm3iMCn2URDLs1U211XfClgkQ
R9a7ojyazwsvfn5A4fT/wq7gLtN2XeMDXa7k9cGH9GMQqsr60K0fo2ezcdWEok8T
iQIH/EA/+J98xpAl9ynUuy+FlVLoNzZ9j0HCLDxc7Qmm2yYQYq5SYV/VtL1fQ930
5TTSKmfSKVB33vA6f2NHhX6g3GA4pYLj0pOGPh/HWBM4HkRvYu2/Ks9gUAnIQS5t
gHEkcRHuUlqGLIfnLT9SWYnp15LedyB3JWjCOXneirILG/ajk98auCSfx36HXezU
8qLSkL8pjKTTAgMBAAGjUzBRMB0GA1UdDgQWBBRDF0QoL+eo7GhriSfqzDiqv5yr
9TAfBgNVHSMEGDAWgBRDF0QoL+eo7GhriSfqzDiqv5yr9TAPBgNVHRMBAf8EBTAD
AQH/MA0GCSqGSIb3DQEBCwUAA4IBAQA81x6FkzFr0ZB9gwQ23wMkvnkPIcy6QOJu
LMmO0aKakep5cY0g649yOZT2kZdo6aceiIM0xg0eG1NzwAyPY5qp6VksPZ0WD7ga
n2BfOTPE6L3P2nKae9R1+SRFAgY1nKH0XK0oN3is2LUsQLew8N5MNfBs1DHTXLrj
LsPC2i/XJ4TVn9AWxm7SKW52VnNZZFqRe5dyntFdMYnuyT3/tUYbZXPXdH/S2Tkt
0StcZ3x06QNgSoCYhAIoKUw58izEkHq6TVXE139ZAlgY1Q7LZKI0UsC9O7+VOLE2
n2bEWErxtEP7ljxdTo1OYYFYeByunlCdgSkcGFBNWN8hXsuNwKFW
-----END CERTIFICATE-----)";

const std::string TEST_KEY = R"(-----BEGIN ENCRYPTED PRIVATE KEY-----
MIIFLTBXBgkqhkiG9w0BBQ0wSjApBgkqhkiG9w0BBQwwHAQIBW71+wLEU6ICAggA
MAwGCCqGSIb3DQIJBQAwHQYJYIZIAWUDBAEqBBD/YucbBzgDyovYId0zhMwgBIIE
0FCaYV3HcT7T2zD8mREDq+d0csJVvTl0w6VSxG1sxi3FVlg+iLOlWI/iWVzkFgE7
0RYdvPy6uPuU38ZBl/UpdcewcisPWABoSFa1Zw+JH1G/orQl0N4czvUHi2UhDcxt
LIYOB3lv3Si2+q+RLAlxOWcQ0eEMCvZYosMIdHwQk0aQMtyup8SmQKbQ9MXXc9Pf
ouc8euklemKeub6yFt2FDFcwdRVUjePTwa/M3GsGeffh+F45bQgXA75uomv6CDMQ
7W7Liz10P3gpXCV6huS69hycIBdn64pxSOtzEXgx39In2s10Hy8hVbQ7tGsNaMuc
BRSM8KxwJJhoTDl6fGW01peL5bSaqBPWjZ0km1X78iRbHXWeLf5Hak7iH/ecNcuD
WG6BcJutAJdftmrr9PSyMAjUplAeJ/azQrHBHLPYm+AejZWYyw8kkekkGNi0guYa
HXa/mhAfJLr6c5MTq96dOVaJPY7BgUgkG5AqVN7xDKfWjh0Beu4nsKouuR1+5e2q
LmgvkcABvrhy0KZYOtl6v8A2NInzuTA3MUbx9pyA+za2N8ADrng/wOzN4wDm2Yp+
zb5gikAPhPy6E5eYnpiIMaC14ivuPfa05wlD3Wzc7Z5lSH7CVM5yHwVDVvM2Yrth
VkhwCP51GD+KjTYYjkgkk4Kgqk9e6nXU4OlyE1qf6Nd7d2Y3Tbb/KzCRePuJ+nQd
zIomylupjKYSPwJy5NwNXNu/hrZDoJXckMGoOzjmx7MIRU9NMmqh/fckXfG9ONlt
R8GIujTLAMALNzJkpKftX3QI2gdQRWec6UpVlGYaxDzyf/zLY1RC76GwFZ4deFd/
9TmTzgmhThPLmnEzq7LZ2W4VOSAs34AHK7g1hfnKFtj7rhDJplfkH2STz1ZbOdef
4rrPfYw+7NKnsDZ1JZQ5HDQDO7fX1U2BpGPEQXWoKKJ6AA6DgnQBnBROnOcHtEpp
zw76XsLape+xCMButVJAhFqCNmSDh7l/aQosgj7IpzhP3MG3NGWasOSBA11j8vgV
YxuaL3SLRrbVzB6rUGcwwSFA7MHXQufreBGjKRimzvoNUJF+Op9yU9zC3kPjJE80
gw3D78r360/TPyz9UxTgQJGvppJHMeUtC8cMZV0SfrfiqUhpkJJKbkDfrMwXln5A
arMo2e3fRK0A/TwbO7iDC0VW+vaCoL95goDNcCRcUpxXzRGh8Dflx+mPP6QA3mhz
qrD+vc5mWKvPS+YzmNBrpQAVGWyRdDTski4y1/vWIfzLkkE1nzTY2CZvjz6OBLnF
jDh2Y0XZ8UwpOUbk7+/8uuSnQm2MtXPvdv+irabyU3iFB+mDcHBBtjimtgWpnWn1
JMT5cwNi/vexbKb/EaQfRo5hHFTbRpUGQTCytK7S/jw/j4IGwBHBZ3eMBW1Vw0hp
XWIWgsA9r6r1CqgeDDl7KZ25r0tf5060YOmFGDbjZeCyhNMORTQVnkDtbsRjAozL
6cy+15erwFXMyaktMbJZxcPtIRj3tHGRlP5I1+7fjrpmm8abMluAEdaLW0ZBQEOx
FgbW9s/FjRHJvthQzDjR50w9nslKHCBBvkQS6RNZlHpPQPE65ZH7hI5oEfD9VfOy
cawrnQZCQZC3CH0WwAKhPYOScDf5xQ5M2K+Q/mLOBUb2
-----END ENCRYPTED PRIVATE KEY-----)";

auto Echo(const Request &req) -> Response {
    return req.data;
}

auto make_server() {
    std::vector<harbour::detail::Ship> ships;
    auto ship_handler = [&](const Request &req, Response &resp) -> asio::awaitable<void> {
        resp = Echo(req);
        co_return;
    };

    auto settings = server::Settings::defaults().with_ssl_data(TEST_CERT, TEST_KEY, "password");

    return server::Server(ship_handler, settings, ships);
}

auto ssl_client(asio::io_context &io_context, const server::Settings &settings) -> asio::awaitable<bool> {
    try {
        auto executor = co_await asio::this_coro::executor;

        // Create SSL context for client
        asio::ssl::context ssl_ctx(asio::ssl::context::sslv23_client);
        ssl_ctx.set_verify_mode(asio::ssl::verify_none);

        // Create SSL stream
        asio::ssl::stream<tcp::socket> socket(executor, ssl_ctx);

        // Connect to server
        tcp::resolver resolver(executor);
        auto endpoints = resolver.resolve("127.0.0.1", std::to_string(settings.port));
        co_await asio::async_connect(socket.lowest_layer(), endpoints, asio::use_awaitable);

        // Perform SSL handshake
        co_await socket.async_handshake(asio::ssl::stream_base::client, asio::use_awaitable);

        // Send request
        co_await async_write(socket, asio::buffer(req), asio::use_awaitable);

        // Read response
        std::array<char, 4096> data;
        auto n   = co_await socket.async_read_some(asio::buffer(data), asio::use_awaitable);
        auto got = std::string_view(data.data(), n);

        co_return got == want;
    } catch (const std::exception &e) {
        log::critical("SSL client exception: {}", e.what());
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
        asio::co_spawn(io_context, [&]() -> asio::awaitable<void> {
                asio::co_spawn(
                    co_await asio::this_coro::executor,
                    srv.listener(),
                    asio::detached
                );
                
                // Run client and get result
                ok = co_await ssl_client(io_context, srv.settings_);
                io_context.stop(); }, asio::detached);

        io_context.run();

        assert(ok);
        return ok ? 0 : 1;

    } catch (const std::exception &e) {
        log::critical("Server exception: {}", e.what());
        return 1;
    }
}