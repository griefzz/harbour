#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <streambuf>
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

// Cache all files in path
auto cache_files(fs::path p) -> Result<FileMap, FileMapError> {
    if (!fs::exists(p)) return Err(FileMapError::FolderNotFound);
    if (!fs::is_directory(p)) return Err(FileMapError::NotAFolder);

    FileMap files;
    for (const auto &entry: fs::recursive_directory_iterator(p)) {
        if (!entry.is_directory()) {
            if (const auto content = read_file(entry); content.has_value()) {
                files[entry.path().filename()] = *content;
            } else {
                return Err(content.error());
            }
        }
    }

    return files;
}