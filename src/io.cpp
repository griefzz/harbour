#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <array>
#if !defined(MSC_VER)
    #include <sys/stat.h>
    #include <ctime>
#endif
#include <harbour/logger.hpp>
#include <harbour/io.hpp>
#include "templates.hpp"

auto FileMapError_string(FileMapError error) noexcept -> std::string {
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

// Read the full contents of a file into a std::string
auto read_file(const fs::path &p) noexcept -> Result<std::string, FileMapError> {
    if (!fs::exists(p)) return Err(FileMapError::FileNotFound);
    if (!fs::is_regular_file(p)) return Err(FileMapError::NotAFile);

    std::ifstream ifs(p);
    if (!ifs) return Err(FileMapError::ReadFailed);

    using It = std::istreambuf_iterator<char>;
    return std::string(It(ifs), It());
}

auto last_modified(const std::string &path) -> std::string {
    auto localtime_xp = [](const std::time_t timer) -> std::tm {
        std::tm bt{};
#if defined(__unix__)
        localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
        localtime_s(&bt, &timer);
#else
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        bt = *std::localtime(&timer);
#endif
        return bt;
    };
#if defined(_MSC_VER)
    return "";
#else
    struct stat result {};
    auto rp = "../../" + path;
    if (stat(rp.c_str(), &result) == 0) {
        const std::time_t modTime = result.st_mtime;
        auto bt                   = localtime_xp(modTime);
        std::array<char, 11> buffer{};// "YYYY-MM-DD\0"
        return {buffer.data(), std::strftime(buffer.data(), buffer.size(), "%Y-%m-%d", &bt)};
    }

    Logger::error("Error getting file stat.\n");
    return "";
#endif
}

// remove target substring from a src string
auto rsstr(const std::string &src, std::string_view target) -> std::string {
    std::string result = src;
    const size_t pos   = result.find(target);
    if (pos != std::string::npos) {
        result.erase(pos, target.length());
    }
    return result;
}

auto create_source_file(const fs::path &path, std::string_view data) noexcept -> std::string {
    // replace characters '<' and '>' that interfere with the rendered html
    std::string escaped(data.begin(), data.end());
    std::size_t pos{};
    while ((pos = escaped.find('<')) != std::string::npos)
        escaped.replace(pos, 1, "&lt;");
    while ((pos = escaped.find('>')) != std::string::npos)
        escaped.replace(pos, 1, "&gt;");

    return std::vformat(harbour_source_item, std::make_format_args(path.string(), escaped));
}

auto create_source_index(const std::vector<fs::path> &src_list) noexcept -> std::string {
    std::string files;
    for (const auto &path: src_list) {
        auto p = path.string();
        std::size_t pos{};
        while ((pos = p.find('\\')) != std::string::npos)
            p.replace(pos, 1, "/");
        files += std::vformat(harbour_source_index_item, std::make_format_args(p, rsstr(p, "/harbour"), last_modified(p)));
    }

    return std::vformat(harbour_source_index_body, std::make_format_args(files));
}

// Cache all files in path
auto cache_files(const fs::path &web_path, const fs::path &root_path) noexcept -> Result<FileMap, FileMapError> {
    if (!fs::exists(web_path)) return Err(FileMapError::FolderNotFound);
    if (!fs::is_directory(web_path)) return Err(FileMapError::NotAFolder);
    if (!fs::exists(root_path)) return Err(FileMapError::FolderNotFound);
    if (!fs::is_directory(root_path)) return Err(FileMapError::NotAFolder);

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

    // Cache src/
    std::vector<fs::path> src_list;
    auto src_path = root_path.string() + "src";
    for (const auto &entry: fs::recursive_directory_iterator(src_path)) {
        if (!entry.is_directory()) {
            if (const auto content = read_file(entry); content.has_value()) {
                auto rel = fs::path("/");
                rel += "harbour/src/" + fs::relative(entry.path(), fs::path(src_path)).string();
                src_list.emplace_back(rel);
                files[rel] = create_source_file(rel, *content);
            } else {
                return Err(content.error());
            }
        }
    }

    // Cache include/
    auto inc_path = root_path.string() + "include";
    for (const auto &entry: fs::recursive_directory_iterator(inc_path)) {
        if (!entry.is_directory()) {
            if (const auto content = read_file(entry); content.has_value()) {
                auto rel = fs::path("/");
                rel += "harbour/include/" + fs::relative(entry.path(), fs::path(inc_path)).string();
                src_list.emplace_back(rel);
                files[rel] = create_source_file(rel, *content);
            } else {
                return Err(content.error());
            }
        }
    }

    std::ranges::sort(src_list);
    files["/harbour/index.html"] = create_source_index(src_list);

    return files;
}