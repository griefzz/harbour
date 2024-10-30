///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file securecookies.cpp
/// @brief Contains the example implementation of a secure cookies encoder with harbour

#include <iostream>
#include <unordered_map>

#include <harbour/harbour.hpp>

using namespace harbour;

// Test certificate and key in PEM format
const std::string EXAMPLE_CERT = R"(-----BEGIN CERTIFICATE-----
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

const std::string EXAMPLE_KEY = R"(-----BEGIN ENCRYPTED PRIVATE KEY-----
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

// index.html
const std::string index_tmpl = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Harbour | Secure Cookies Example</title>
</head>
<body>
    <h1>Cookies:</h1>
    {}
</body>
</html>)";

struct Session {
    SecureCookies &securecookies;///< Secure cookie encoder

    auto operator()(const Request &req) -> Response {
        // Create a Cookies object from a request.
        // When creating a Cookies object from another source
        // we need to handle the possibility of a parsing error
        if (auto cookies = Cookies::create(req)) {
            // Grab the current session if it exists
            if (auto data = cookies->get("example-session")) {
                // Decode the session data into our existing map
                if (securecookies.decode("example-session", *data, cookies->data)) {
                    // Show the user the encoded and decoded cookies
                    return tmpl::render(index_tmpl, cookies->string());
                }
            }
        }

        // Create an empty Cookie object to store our cookie data
        // Any object that can be used with msgpack::{pack,unpack} works
        cookies::Map data;
        data["id"]   = "123";
        data["name"] = "bob";

        // Encode the cookies
        auto encoded = securecookies.encode("example-session", data);
        if (encoded.empty()) {
            log::warn("Failed to encode cookies!");
            return http::Status::InternalServerError;
        }

        // Create a new Cookies object and store the encoded data
        auto cookies = Cookies::create()
                               .with_http_only()                                       // Only allow HTTP Responses to set cookies
                               .with_secure()                                          // Only allow this cookie on SSL connections
                               .with_same_site_policy(cookies::SameSitePolicy::Strict);// Only allow this cookie on the origin domain

        // Store the encoded session data
        cookies["example-session"] = encoded;

        // Respond to the client including our cookies
        log::info("Setting cookies: {}", cookies.string());
        return Response(http::Status::OK)
                .with_cookies(cookies)
                .with_data(tmpl::render(index_tmpl, "Reload to show cookies!"));
    }
};

auto main() -> int {
    // Create a secure cookies encoder
    auto securecookies = SecureCookies::create();
    if (!securecookies) {
        log::critical("Failed to create secure cookie storage!");
        return 1;
    }

    auto settings = server::Settings::defaults()
                            .with_ssl_data(EXAMPLE_CERT, EXAMPLE_KEY, "password");

    Harbour hb(settings);
    // Create/show secure cookies to the user
    hb.dock("/", Session{*securecookies});// Pass in our secure cookie encoder as Cargo
    hb.sail();
    return 0;
}