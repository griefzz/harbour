#pragma once
#include <harbour/logger.hpp>
#include <harbour/io.hpp>
#include <harbour/result.hpp>
#include "config.hpp"

enum class CacheError {
    NotFound
};

class Cache {
public:
    Cache() {
        if (auto cache = cache_files(ServerWebPath, ServerSrcPath)) {
            files = *cache;
        } else {
            Logger::error(std::format("Failed to initialize cache: {}\n", FileMapError_string(cache.error())));
            abort();
        }
    }

    using Key = FileMap::key_type;
    using T   = FileMap::mapped_type;

    // returns an error if there isnt a value in the cache
    auto operator[](const Key &key) noexcept -> Result<T, CacheError> {
        if (auto s = files.find(key); s != files.end())
            return s->second;
        else
            return Err(CacheError::NotFound);
    }

    // returns an error if there isnt a value in the cache
    auto operator[](Key &&key) noexcept -> Result<T, CacheError> {
        if (auto s = files.find(key); s != files.end())
            return s->second;
        else
            return Err(CacheError::NotFound);
    }

private:
    FileMap files;
};