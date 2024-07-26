///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///

#include <iostream>
#include <format>
#include <cassert>

#include <harbour/harbour.hpp>

static const std::string valid_cookie =
        "GET /api/v1/foo HTTP/1.1\r\n"
        "Host: github.com\r\n"
        "Connection: keep-alive\r\n"
        "Cache-Control: max-age=0\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/535.11 (KHTML, like Gecko) Chrome/17.0.963.56 Safari/535.11\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Accept-Encoding: gzip,deflate,sdch\r\n"
        "Accept-Language: zh-CN,zh;q=0.8,en;q=0.6\r\n"
        "Accept-Charset: gb18030,utf-8;q=0.7,*;q=0.3\r\n"
        "Cookie: id=123; name=bob; HttpOnly; Secure\r\n"
        "If-None-Match: 7f9c6a2baf61233cedd62ffa906b604f\r\n"
        "\r\n";


static const std::string invalid_cookie =
        "GET /api/v1/foo HTTP/1.1\r\n"
        "Host: github.com\r\n"
        "Connection: keep-alive\r\n"
        "Cache-Control: max-age=0\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/535.11 (KHTML, like Gecko) Chrome/17.0.963.56 Safari/535.11\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Accept-Encoding: gzip,deflate,sdch\r\n"
        "Accept-Language: zh-CN,zh;q=0.8,en;q=0.6\r\n"
        "Accept-Charset: gb18030,utf-8;q=0.7,*;q=0.3\r\n"
        "Cookie: id=123; name=bob; HttpOnly; DoesntExist; Secure\r\n"
        "If-None-Match: 7f9c6a2baf61233cedd62ffa906b604f\r\n"
        "\r\n";

auto main() -> int {
    std::shared_ptr<harbour::server::Socket> sock;

    // Test a valid cookie
    if (auto req = harbour::Request::create(sock, valid_cookie.data(), valid_cookie.size())) {
        if (auto cookie = harbour::Cookies::create(*req)) {
            if (auto id = cookie->get("id")) {
                assert(id == "123");
            } else {
                assert(false);
                return 1;
            }

            if (auto name = cookie->get("name")) {
                assert(name == "bob");
            } else {
                assert(false);
                return 1;
            }

            assert(cookie->flags.is_secure == true);
            if (cookie->flags.is_secure != true)
                return 1;

            assert(cookie->flags.is_http_only == true);
            if (cookie->flags.is_http_only != true)
                return 1;
        }
    }

    // Test an invalid cookie key=value
    if (auto req = harbour::Request::create(sock, invalid_cookie.data(), invalid_cookie.size())) {
        if (auto cookie = harbour::Cookies::create(*req)) {
            assert(false);// cookie shouldnt be valid!
            return 1;
        }
    }

    return 0;
}