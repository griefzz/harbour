///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file websockets.cpp
/// @brief Contains the example implementation of harbours websockets

#include <string>

#include <harbour/harbour.hpp>

using namespace harbour;

auto Index() -> awaitable<Response> {
    co_return R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Harbour | WebSocket Example</title>
    <script>  
    window.addEventListener("load", function(evt) {

        var output = document.getElementById("output");
        var input = document.getElementById("input");
        var ws;

        var print = function(message) {
            var d = document.createElement("div");
            d.textContent = message;
            output.appendChild(d);
            output.scroll(0, output.scrollHeight);
        };

        document.getElementById("open").onclick = function(evt) {
            if (ws) { return false; }
            ws = new WebSocket("ws://127.0.0.1:8080/echo");
            ws.onopen = function(evt) { print("OPEN"); }
            ws.onclose = function(evt) {
                print("CLOSE");
                ws = null;
            }
            ws.onmessage = function(evt) { print("RESPONSE: " + evt.data); }
            ws.onerror = function(evt) { print("ERROR: " + evt.data); }
            return false;
        };

        document.getElementById("send").onclick = function(evt) {
            if (!ws) { return false; }
            print("SEND: " + input.value);
            ws.send(input.value);
            return false;
        };

        document.getElementById("close").onclick = function(evt) {
            if (!ws) { return false; }
            ws.close();
            return false;
        };

    });
    </script>
</head>
<body>
    <table>
        <tr><td valign="top" width="50%">
            <p>Click "Open" to create a connection to the server, 
            "Send" to send a message to the server and "Close" to close the connection. 
            You can change the message and send multiple times.
            <p>
        <form>
            <button id="open">Open</button>
            <button id="close">Close</button>
            <p><input id="input" type="text" value="Hello world!">
            <button id="send">Send</button>
        </form>
        </td><td valign="top" width="50%">
        <div id="output" style="max-height: 70vh;overflow-y: scroll;"></div>
    </td></tr></table>
</body>
</html>)";
}

auto Echo(const Request &req) -> awaitable<std::optional<Response>> try {
    // Upgrade to a websocket connection
    if (auto ws = co_await websocket::upgrade(req)) {
        log::info("WebSocket connection open");

        for (;;) {
            // Read message
            auto msg = co_await ws->read();
            if (!msg) break;// Connection closed normally

            // Echo message back
            co_await ws->send(*msg);
        }

        log::info("WebSocket connection closed");
        co_return std::nullopt;
    }

    co_return http::Status::BadRequest;
} catch (const asio::system_error &e) {
    log::warn("WebSocket error: {}", e.what());
    co_return http::Status::BadRequest;
}

auto main() -> int {
    Harbour hb;
    hb.dock("/", Index).dock("/echo", Echo);
    hb.sail();
    return 0;
}
