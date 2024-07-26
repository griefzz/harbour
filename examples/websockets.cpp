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

auto Echo(const Request &req) -> awaitable<Response> try {
    // Upgrade to a websocket connection
    if (auto ws = co_await websocket::upgrade(req)) {
        // Send clients a copy of their request
        log::info("WebSocket connection open");
        for (;;) {
            auto msg = co_await ws->read(4096); // Read a maximum of 4096 bytes
            auto n   = co_await ws->write(msg); // Send back the msg returning bytes sent
        }
    } else {
        // Correctly respond when an upgrade fails
        co_return http::Status::BadRequest;
    }

    // Websocket connection automatically closes itself here
    co_return http::Status::OK;
} catch (const asio::system_error &e) {
    // Handle any coroutine errors

    // this is a normal exception for when the connection closes
    // while trying to read (client disconnected)
    if (e.code() == asio::error::eof) {
        log::info("WebSocket connection closed");
        co_return http::Status::OK;
    }

    // Any other exception gets handled here
    log::warn("Websocket Exception: {}", e.what());
    co_return http::Status::BadRequest;
}

int main() {
    Harbour hb;
    hb.dock("/", Index).dock("/echo", Echo);
    hb.sail();
    return 0;
}
