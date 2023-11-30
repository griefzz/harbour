#pragma once
#include <vector>
#include <string_view>
#include <harbour/io.hpp>

#if HARBOUR_ENABLE_COMPRESSION
/// @brief Path to x509 certificate
static auto ServerCertificatePath() -> const std::string & {
    static const std::string cert_path = "/home/stone/harbour/cert.pem";
    return cert_path;
}

/// @brief Path to TLS private key
static auto ServerPrivateKeyPath() -> const std::string & {
    static const std::string key_pay = "/home/stone/harbour/key.pem";
    return key_pay;
}
#endif

/// @brief Path to our files to serve
static auto ServerRelPath() -> const std::string & {
#if defined(MSC_VER)
    static const std::string rel_path = "../../";
#else
    static const std::string rel_path = "../";
#endif
    return rel_path;
}

/// @brief Path to our files to serve
static auto ServerWebPath() -> const std::string & {
    static const std::string web_path = ServerRelPath() + "web";
    return web_path;
}

/// @brief Name of our server
static auto ServerName() -> const std::string & {
    static const std::string name = "Harbour";
    return name;
}

/// @brief Port our server runs on
static constexpr uint32_t ServerPort = 8080;

/// @brief Server version
static auto ServerVersion() -> const std::string & {
    static const std::string version = read_file(ServerRelPath() + "VERSION").value_or("null");
    return version;
}

/// @brief 404 File path
static auto Server404Path() -> const std::string & {
    static const std::string path = "/404.html";
    return path;
}

/// @brief Accepted MIME Types
using MimeTypes = std::vector<std::pair<std::vector<std::string>, std::string>>;
static auto ServerAcceptedMimeTypes() -> const MimeTypes & {
    static const MimeTypes mimes = {{{".txt", ".text", ".conf", ".log", ".ini"}, "text/plain"},
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

    return mimes;
}
