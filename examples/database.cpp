///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file database.cpp
/// @brief Contains the example implementation of a database with harbour

#include <iostream>
#include <unordered_map>

#include <harbour/harbour.hpp>

using namespace harbour;

using Database               = std::unordered_map<std::string, std::string>;
const std::string index_tmpl = R"(<!DOCTYPE html>
 <html lang="en">
 <head>
     <meta charset="UTF-8">
     <meta name="viewport" content="width=device-width, initial-scale=1.0">
     <title>Harbour | Database Example</title>
 </head>
 <body>
     <h1>Users:</h1>
     {}
     <br>
     <h1>Add User</h1>
     <form action="/api/v1/user/add" method="POST">
         <label for="name">Name:</label>
         <input type="text" id="name" name="name" required><br><br>

         <label for="email">Email:</label>
         <input type="email" id="email" name="email" required><br><br>

         <input type="submit" value="Submit">
     </form>
 </body>
 </html>)";

struct Index {
    auto operator()() {
        std::string users;
        for (const auto &entry: db)
            users += tmpl::render("<p><b>Name: </b>{} <b>Email: </b>{}</p>", entry.first, entry.second);
        return tmpl::render(index_tmpl, users);
    }

    Database &db;
};

struct NewUser {
    auto operator()(const Request &req) {
        auto user  = req.form("name");
        auto email = req.form("email");
        if (user && email) {
            db[std::string(*user)] = std::string(*email);
        }

        return Response().with_redirect("/");
    }

    Database &db;
};

auto main() -> int {
    Database db;// Connect to your database
    Harbour hb;
    hb.dock("/", Index{db})                        // Pass in the database to your ship
            .dock("/api/v1/user/add", NewUser{db});// Add new users to the db with an API
    hb.sail();
    return 0;
}