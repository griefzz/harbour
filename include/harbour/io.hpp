#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include "result.hpp"

namespace fs = std::filesystem;

// Error types for file and folder scanning
enum class FileMapError { FileNotFound,
                          FolderNotFound,
                          NotAFolder,
                          NotAFile,
                          ReadFailed };

// convert a FileMapError to a string
auto FileMapError_string(FileMapError error) noexcept -> std::string;

// Hashmap containing file paths as keys and their content as the value
using FileMap = std::unordered_map<fs::path, std::string>;

// Read the full contents of a file into a std::string
auto read_file(fs::path p) noexcept -> Result<std::string, FileMapError>;

// Cache all files in path web_path and src_path
auto cache_files(fs::path web_path, fs::path src_path = "") noexcept -> Result<FileMap, FileMapError>;