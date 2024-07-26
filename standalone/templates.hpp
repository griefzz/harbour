///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///

#pragma once

#include <string>

#include <harbour/harbour.hpp>

namespace tmpls {

    using namespace std::literals;

    constexpr auto index() -> const char * {
        return R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
    html {{
        max-width: 70ch;
        padding: 3em 1em;
        margin: auto;
        line-height: 1.75;
        font-size: 1.25em;
    }}

    h1,h2,h3,h4,h5,h6 {{
        margin: 3em 0 1em;
    }}

    p,ul,ol {{
      margin-bottom: 2em;
      color: #1d1d1d;
      font-family: sans-serif;
    }}
    </style>
    <title>{0}</title>
</head>
<body>
    <h1>Directory listing for {0}</h1>
    <hr>
    <ul>
        {1}
    </ul>
    <hr>
    <footer><center>Harbour 0.1.0</center></footer>
</body>
</html>)";
    }

    constexpr auto file() -> const char * {
        return "\t\t<li><a href=\"{}\">{}</a></li>\n";
    }

};// namespace tmpls