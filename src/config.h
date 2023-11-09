#pragma once
#include <string_view>
#include "io.h"

// Path to our files to serve
std::string ServerWebPath = "../../web";

// Name of our server
std::string ServerName = "TestServer";

// Port our server runs on
constexpr uint32_t ServerPort = 8080;

// Server version
std::string ServerVersion = read_file("../../VERSION").value_or("null");

// Accepted MIME Types
std::vector<std::pair<std::vector<std::string>, std::string>> ServerAcceptedMimeTypes = {
        {{".txt", ".text", ".conf", ".log", ".ini"}, "text/plain"},
        {{".html", ".htm"}, "text/html"},
        {{".pdf"}, "application/pdf"},
        {{".jpg", ".jpeg"}, "image/jpeg"},
        {{".png"}, "image/png"},
        {{".gif"}, "image/gif"},
        {{".mp3"}, "audio/mpeg"},
        {{".mp4"}, "video/mp4"},
        {{".json"}, "application/json"},
        {{".xml"}, "application/xml"},
        {{".zip"}, "application/zip"},
        {{".css"}, "text/css"},
        {{".doc", ".docx"}, "application/msword"},
        {{".xls", ".xlsx"}, "application/vnd.ms-excel"},
        {{".ppt", ".pptx"}, "application/vnd.ms-powerpoint"},
        {{".wav"}, "audio/wav"},
        {{".avi"}, "video/x-msvideo"},
        {{".bmp"}, "image/bmp"},
        {{".csv"}, "text/csv"},
        {{".js"}, "application/javascript"},
        {{".ico"}, "image/x-icon"},
        {{".svg", ".svgz"}, "image/svg+xml"}};
