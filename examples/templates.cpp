///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file templates.cpp
/// @brief Contains the example implementation of harbours templating

#include <harbour/harbour.hpp>

using namespace harbour;

// Log a message to the console
auto Hello() {
    log::info(tmpl::render("Hello from {}!", "Harbour"));
}

// Load and render a template
auto Render() {
    log::info("Rendering template...");
    // First parameter is the file to render
    // The other parameters are the arguments to use for rendering
    return tmpl::render_file("index.tmpl", "Harbour", "<h1>Ahoy!</h1>")
            .value_or("Couldn't compile template.");
}

// Load a file asynchronously and return it to the client
auto AsyncLoad() -> awaitable<Response> {// Use an awaitable<Response> to get access to co_await/co_return
    log::info("Async loading file...");

    // Await the result of an async load_file
    // First parameter is the file to load
    // Second parameter is the completion token
    auto file = co_await tmpl::load_file_async("index.html", use_awaitable);

    // Return the result if file was found, otherwise return an error message
    co_return file.value_or("Failed to load file");
}

// Render a file asynchronously and return it to the client
auto AsyncRender() -> awaitable<Response> {
    log::info("Async rendering template...");

    // Await the result of an async render_file
    // First parameter is the completion handler,
    // Second parameter is file to render
    // Third parameter is the arguments to pass in for rendering
    auto file = co_await tmpl::render_file_async(use_awaitable, "index.tmpl", "Hello!");

    // Return the result if file was found, otherwise return an error message
    co_return file.value_or("Couldn't compile template.");
}

auto main() -> int {
    Harbour hb;
    hb.dock("/hello", Hello)
      .dock("/render", Render)
      .dock("/async/load", AsyncLoad)
      .dock("/async/render", AsyncRender);
    hb.sail();
    return 0;
}