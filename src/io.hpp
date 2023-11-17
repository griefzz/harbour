#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <algorithm>
#if !defined(_WIN32)
    #include <sys/stat.h>
    #include <ctime>
#endif
#include "result.hpp"

namespace fs = std::filesystem;

// Error types for file and folder scanning
enum class FileMapError { FileNotFound,
                          FolderNotFound,
                          NotAFolder,
                          NotAFile,
                          ReadFailed };

auto fme_to_string(FileMapError error) {
    switch (error) {
        case FileMapError::FileNotFound:
            return "File not found";
        case FileMapError::FolderNotFound:
            return "Folder not found";
        case FileMapError::NotAFolder:
            return "Not a folder";
        case FileMapError::NotAFile:
            return "Not a file";
        case FileMapError::ReadFailed:
            return "Read failed";
        default:
            return "Unknown FileMapError";
    }
}

// Hashmap containing file paths as keys and their content as the value
using FileMap = std::unordered_map<fs::path, std::string>;

// Read the full contents of a file into a std::string
auto read_file(fs::path p) noexcept -> Result<std::string, FileMapError> {
    if (!fs::exists(p)) return Err(FileMapError::FileNotFound);
    if (!fs::is_regular_file(p)) return Err(FileMapError::NotAFile);

    std::ifstream ifs(p);
    if (!ifs) return Err(FileMapError::ReadFailed);

    using It = std::istreambuf_iterator<char>;
    return std::string(It(ifs), It());
}

std::string last_modified(const std::string &path) {
#if _WIN32
    return "";
#else
    struct stat result;
    auto rp = ".." + path;
    if (stat(rp.c_str(), &result) == 0) {
        std::time_t modTime = result.st_mtime;
        char buffer[11];// "YYYY-MM-DD\0"
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", std::localtime(&modTime));
        return buffer;
    } else {
        Logger::error("Error getting file stat.\n");
        return "";
    }
#endif
}

auto create_source_file(fs::path path, std::string_view data) noexcept -> std::string {
    // replace characters '<' and '>' that interfere with the rendered html
    std::string escaped(data.begin(), data.end());
    std::size_t pos;
    while ((pos = escaped.find("<")) != std::string::npos)
        escaped.replace(pos, 1, "&lt;");
    while ((pos = escaped.find(">")) != std::string::npos)
        escaped.replace(pos, 1, "&gt;");

    std::string header = "<!DOCTYPE html><meta charset=UTF-8><title>" + path.string() +
                         "</title><meta content=\"width=device-width,initial-scale"
                         "=1\"name=viewport><link href=https://cdn.jsdelivr.net/npm/highlight.js@10.7.2/styles/nord.css rel=st"
                         "ylesheet><script src=https://unpkg.com/@highlightjs/cdn-assets@11.9.0/highlight.min.js></script><scr"
                         "ipt src=//cdnjs.cloudflare.com/ajax/libs/highlightjs-line-numbers.js/2.8.0/highlightjs-line-numbers."
                         "min.js></script><script>hljs.highlightAll(),hljs.initLineNumbersOnLoad()</script><style>body,html{pa"
                         "dding:0;margin:auto;font-size:1.1em;height:100%;width:100%;background:#0d1117}.breadcrumb{background"
                         ":#161b22;font-size:.7em;color:gray}.hljs{background:#0d1117}.hljs-ln-numbers{-webkit-touch-callout:n"
                         "one;-webkit-user-select:none;-khtml-user-select:none;-moz-user-select:none;-ms-user-select:none;user"
                         "-select:none;text-align:center;color:#ccc;border-right:1px solid #ccc;vertical-align:top;padding-rig"
                         "ht:5px}.hljs-ln-code{padding-left:10px}</style><div class=breadcrumb><a href=/src/ >Back</a> - " +
                         path.string() + "<hr></div><pre><code class=language-cpp>";

    std::string footer = "</code></pre></body></html>";

    return header + escaped + footer;
}

auto create_source_index(std::vector<fs::path> src_list) noexcept -> std::string {
    std::string header = "<!DOCTYPE html><html lang=en><meta charset=UTF-8><meta content=\"width=device-width,initial-scale=1\"name="
                         "viewport><link href=https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css rel=stylesheet><"
                         "title>Harbour Source Code</title><style>.header-bg{background-color:#161b22}.body-bg{background-color:#0d1"
                         "117}</style><body class=\"body-bg p-4 text-white\"><div class=\"max-w-lg mx-auto\"><div class=\"border bor"
                         "der-gray-700 header-bg p-0 rounded-md shadow-md\"><div class=\"flex items-center justify-between mb-1 ml-1"
                         " mr-1 p-1\"><a href=https://github.com/griefzz/harbour target=_blank><img alt=\"GitHub Logo\"class=\"h-6 w-6\"src="
                         "/github-mark-white.svg></a><h2 class=\"font-semibold text-2xl\">Harbour</h2><a href=/ class=\"font-semibold"
                         " text-1xl\"><button class=\"hover:underline text-blue-400\">Home</button></a></div>";

    constexpr std::string_view source_file_format = "<div class=\"body-bg border border-gray-700 flex items-center justify-between mb-0 p-2\""
                                                    "><a href={}><p class=\"text-sm hover:text-blue-400 hover:underline\">{}</p></a><p class="
                                                    "\"text-sm text-gray-400\">{}</div>";

    std::string index;
    for (auto &path: src_list) {
        auto p = path.string();
        std::size_t pos;
        while ((pos = p.find("\\")) != std::string::npos)
            p.replace(pos, 1, "/");
        index += std::format(source_file_format, p, p, last_modified(p));
    }

    auto footer = "</div></div></body></html>";

    return header + index + footer;
}

// Cache all files in path
auto cache_files(fs::path web_path, fs::path src_path) noexcept -> Result<FileMap, FileMapError> {
    if (!fs::exists(web_path)) return Err(FileMapError::FolderNotFound);
    if (!fs::is_directory(web_path)) return Err(FileMapError::NotAFolder);
    if (!fs::exists(src_path)) return Err(FileMapError::FolderNotFound);
    if (!fs::is_directory(src_path)) return Err(FileMapError::NotAFolder);

    FileMap files;

    // Cache our web files
    for (const auto &entry: fs::recursive_directory_iterator(web_path)) {
        if (!entry.is_directory()) {
            if (const auto content = read_file(entry); content.has_value()) {
                // trim off the actual path to the directory
                auto rel = fs::path("/");
                rel += fs::relative(entry.path(), fs::path(web_path));
                files[rel] = *content;
            } else {
                return Err(content.error());
            }
        }
    }

    // Cache and create our source code files
    std::vector<fs::path> src_list;
    for (const auto &entry: fs::recursive_directory_iterator(src_path)) {
        if (!entry.is_directory()) {
            if (const auto content = read_file(entry); content.has_value()) {
                auto rel = fs::path("/");
                rel += "src/" + fs::relative(entry.path(), fs::path(src_path)).string();
                src_list.emplace_back(rel);
                files[rel] = create_source_file(rel, *content);
            } else {
                return Err(content.error());
            }
        }
    }

    files["/src/index.html"] = create_source_index(src_list);

    return files;
}