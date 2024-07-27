///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file forms.cpp
/// @brief Contains the example implementation of harbours form data handling

#include <harbour/harbour.hpp>

using namespace harbour;

auto Index() {
    return R"(<!DOCTYPE html>
 <html lang="en">
 <head>
     <meta charset="UTF-8">
     <meta name="viewport" content="width=device-width, initial-scale=1.0">
     <title>Harbour | POST Example</title>
 </head>
 <body>
     <h1>Example POST Form</h1>
     <form action="/api/v1/post" method="POST">
         <label for="name">Name:</label>
         <input type="text" id="name" name="name" required><br><br>

         <label for="email">Email:</label>
         <input type="email" id="email" name="email" required><br><br>

         <input type="submit" value="Submit">
     </form>
 </body>
 </html>)";
}

auto Post(const Request &req) -> Response {
    if (req.method == http::Method::POST) {
        auto name  = req.form("name").value_or("null");
        auto email = req.form("email").value_or("null");
        return tmpl::render("<h1>Name: {}</h1><p><h2>Email: {}</h2>", name, email);
    }

    return Response().with_redirect("/");
}

auto main() -> int {
    Harbour hb;
    hb.dock("/", Index).dock("/api/v1/post", Post);
    hb.sail();
    return 0;
}