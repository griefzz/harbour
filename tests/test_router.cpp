#include "testing.h"
#include "server.h"

TEST(Router, OK) {
    const auto req = "GET /index.html HTTP/1.1\n"
                     "Host: www.example.com\n\n";
    Server server;
    auto resp = server.request_handler(req);
    EXPECT_EQ(resp.contains("HTTP/1.1 200 OK"), true);
}

TEST(Router, default_index) {
    const auto req = "GET / HTTP/1.1\n"
                     "Host: www.example.com\n\n";
    Server server;
    auto resp = server.request_handler(req);
    EXPECT_EQ(resp.contains("HTTP/1.1 200 OK"), true);
}

TEST(Router, FileNotFound) {
    const auto req = "GET /doesnotexist.html HTTP/1.1\n"
                     "Host: www.example.com\n\n";
    Server server;
    auto resp = server.request_handler(req);
    EXPECT_EQ(resp.contains("HTTP/1.1 500 Internal Server Error"), true);
}

TEST(Router, invalid) {
    const auto req = "GET  HTTP/1.1\n"
                     "Host: www.example.com\n\n";
    Server server;
    auto resp = server.request_handler(req);
    EXPECT_EQ(resp.contains("HTTP/1.1 500 Internal Server Error"), true);
}

auto main() -> int {
    RUN_ALL_TESTS();

    return 0;
}