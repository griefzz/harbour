#pragma once
#include <vector>
#include <string_view>
#include <harbour/io.hpp>

// Enable compression when on linux
#if _WIN32
    #define HARBOUR_ENABLE_COMPRESSION false
#else
    #define HARBOUR_ENABLE_COMPRESSION true
#endif

#if !_WIN32
// Path to x509 certificate
static std::string ServerCertificatePath = "/home/stone/harbour/cert.pem";

// Path to private key
static std::string ServerPrivateKeyPath = "/home/stone/harbour/key.pem";
#endif

// Fixup for relative pathing
#if _WIN32
static std::string ServerRelPath = "../../";
#else
static std::string ServerRelPath = "../";
#endif

// Path to our files to serve
static std::string ServerWebPath = ServerRelPath + "web";

// Name of our server
static std::string ServerName = "TestServer";

// Port our server runs on
static constexpr uint32_t ServerPort = 8080;

// Server version
static std::string ServerVersion = read_file(ServerRelPath + "VERSION").value_or("null");

// Accepted MIME Types
static std::vector<std::pair<std::vector<std::string>, std::string>> ServerAcceptedMimeTypes = {
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
        {{".svg", ".svgz"}, "image/svg+xml"},
        {{".c", ".cpp", ".h", ".hpp"}, "text/html"}};
