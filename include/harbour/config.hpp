#pragma once
#include <vector>
#include <string_view>
#include <harbour/io.hpp>

#if !_WIN32
/// @brief Path to x509 certificate
static std::string ServerCertificatePath = "/home/stone/harbour/cert.pem";

/// @brief Path to TLS private key
static std::string ServerPrivateKeyPath = "/home/stone/harbour/key.pem";
#endif

#if _WIN32
/// @brief Path to our files to serve
static std::string ServerRelPath = "../../";
#else
/// @brief Path to our files to serve
static std::string ServerRelPath = "../";
#endif

/// @brief Path to our files to serve
static std::string ServerWebPath = ServerRelPath + "web";

/// @brief Name of our server
static std::string ServerName = "Harbour";

/// @brief Port our server runs on
static constexpr uint32_t ServerPort = 8080;

/// @brief Server version
static std::string ServerVersion = read_file(ServerRelPath + "VERSION").value_or("null");

/// @brief 404 File path
static std::string Server404Path = "/404.html";

/// @brief Accepted MIME Types
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
