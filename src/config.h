#pragma once
#include <string_view>
#include "io.h"

// Path to our files to serve
std::string_view ServerWebPath = "../../web";

// Name of our server
std::string_view ServerName = "TestServer";

// Server version
std::string_view ServerVersion = read_file("../../VERSION").value_or("null");
