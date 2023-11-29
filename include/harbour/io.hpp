/// @file io.hpp
///
/// @brief Header file defining operations for file and folder scanning with error handling.
///
/// This file contains the declarations for functions to read and cache file contents, as well as
/// the necessary types and error enumerations used for file operations.
#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include "result.hpp"

namespace fs = std::filesystem;

/// @enum FileMapError
/// @brief Enumerates possible error types for file and folder scanning operations.
/// This enumeration lists the types of errors that can occur when attempting to read files
/// or scan directories.
enum class FileMapError {
    FileNotFound,  ///< Indicates that the specified file was not found.
    FolderNotFound,///< Indicates that the specified folder was not found.
    NotAFolder,    ///< Indicates that the specified path is not a folder.
    NotAFile,      ///< Indicates that the specified path is not a file.
    ReadFailed     ///< Indicates that reading the file has failed.
};

/// @brief Converts a FileMapError to a human-readable string.
/// @param error The error to convert.
/// @return A string representation of the given FileMapError.
auto FileMapError_string(FileMapError error) noexcept -> std::string;

/// @brief Type definition for a hashmap containing file paths and their contents.
/// This hashmap uses file paths as keys and stores the corresponding file contents as values.
using FileMap = std::unordered_map<fs::path, std::string>;

/// @brief Reads the full contents of a file into a string.
/// @param p The file path to read from.
/// @return A Result object containing the file contents as a string or a FileMapError.
auto read_file(fs::path p) noexcept -> Result<std::string, FileMapError>;

/// @brief Caches all files in the specified web_path and optionally in src_path.
/// @param web_path Path to web directory
/// @param src_path Path to source code of harbour
auto cache_files(fs::path web_path, fs::path src_path = "") noexcept -> Result<FileMap, FileMapError>;