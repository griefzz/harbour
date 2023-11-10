#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <algorithm>
#include "result.h"

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
auto read_file(fs::path p) -> Result<std::string, FileMapError> {
    if (!fs::exists(p)) return Err(FileMapError::FileNotFound);
    if (!fs::is_regular_file(p)) return Err(FileMapError::NotAFile);

    std::ifstream ifs(p);
    if (!ifs) return Err(FileMapError::ReadFailed);

    using It = std::istreambuf_iterator<char>;
    return std::string(It(ifs), It());
}

auto create_source_file(fs::path path, std::string_view data) -> std::string {
    // replace characters that interfere with the rendered html
    std::string escaped(data.begin(), data.end());
    std::size_t pos;
    while ((pos = escaped.find("<")) != std::string::npos)
        escaped.replace(pos, 1, "&lt;");
    while ((pos = escaped.find(">")) != std::string::npos)
        escaped.replace(pos, 1, "&gt;");
    std::string header = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>" + path.string() +
                         "</title>"
                         "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><link r"
                         "el=\"stylesheet\" href=\"https://unpkg.com/highlight.js@10.7.2/styles/a11y-dar"
                         "k.css\"><script src=\"https://unpkg.com/@highlightjs/cdn-assets@11.9.0/highlig"
                         "ht.min.js\"></script><script src=\"//cdnjs.cloudflare.com/ajax/libs/highlightj"
                         "s-line-numbers.js/2.8.0/highlightjs-line-numbers.min.js\"></script><script>hlj"
                         "s.highlightAll(),hljs.initLineNumbersOnLoad()</script><style>body,html{padding"
                         ":0;margin:auto;font-size:1.1em;height:100%;width:100%;background:#2b2b2b}.brea"
                         "dcrumb{font-size:.7em;color:gray}.hljs-ln-numbers{-webkit-touch-callout:none;-"
                         "webkit-user-select:none;-khtml-user-select:none;-moz-user-select:none;-ms-user"
                         "-select:none;user-select:none;text-align:center;color:#ccc;border-right:1px so"
                         "lid #ccc;vertical-align:top;padding-right:5px}.hljs-ln-code{padding-left:10px}"
                         "</style></head><body><div class=\"breadcrumb\"><a href=\"/src/\">Back</a>- " +
                         path.string() + "<hr></div><pre><code class=\"language-cpp\">";
    std::string footer = "</code></pre></body></html>";

    return header + escaped + footer;
}

auto create_source_index(std::vector<fs::path> src_list) -> std::string {
    std::string header = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Source Code</title><meta"
                         "name=\"viewport\" content=\"width=device-width,initial-scale=1\"><style>body,html{p"
                         "adding:0;margin:auto;font-size:1.1em;height:100%;width:100%;background:#2b2b2b;colo"
                         "r:gray}.breadcrumb{font-size:.7em;color:gray}</style></head><body><div class=\"brea"
                         "dcrumb\"><a href=\"/src/\">Home</a><hr></div><div id=\"content\"></div><script src="
                         "\"https://cdn.jsdelivr.net/npm/marked/marked.min.js\"></script><script>table = `\n";
    std::string index;

    index += "Source code\n===========\n";
    for (auto &path: src_list) {
        auto p = path.string();
        std::size_t pos;
        while ((pos = p.find("\\")) != std::string::npos)
            p.replace(pos, 1, "/");
        index += std::format("- [{}]({})\n", p, "/" + p);
    }
    auto footer = "`;\ndocument.getElementById('content').innerHTML = marked.parse(table);</script></body></html>";

    return header + index + footer;
}

// Cache all files in path
auto cache_files(fs::path web_path, fs::path src_path) -> Result<FileMap, FileMapError> {
    fs::path posix_path("a/b/c");
    posix_path.make_preferred();
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
                auto rel   = fs::relative(entry.path(), fs::path(web_path));
                files[rel] = *content;
            } else {
                return Err(content.error());
            }
        }
    }


    // Cache our web files
    std::vector<fs::path> src_list;
    for (const auto &entry: fs::recursive_directory_iterator(src_path)) {
        if (!entry.is_directory()) {
            if (const auto content = read_file(entry); content.has_value()) {
                fs::path rel = "src/" + fs::relative(entry.path(), fs::path(src_path)).string();
                src_list.emplace_back(rel);
                files[rel] = create_source_file(rel, *content);
            } else {
                return Err(content.error());
            }
        }
    }

    files["src/index.html"] = create_source_index(src_list);

    return files;
}