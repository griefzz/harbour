#pragma once
#include "logger.h"
#include "io.h"
#include "config.h"
#include "result.h"

enum class CacheError {
    NotFound
};

class Cache {
public:
    Cache() {
        if (auto cache = cache_files(ServerWebPath, ServerSrcPath); cache.has_value()) {
            files = *cache;
        } else {
            Logger::error(std::format("Failed to initialize cache: {}\n", fme_to_string(cache.error())));
            abort();
        }
    }

    using Key = FileMap::key_type;
    using T   = FileMap::mapped_type;

    // returns an error if there isnt a value in the cache
    auto operator[](const Key &key) -> Result<T, CacheError> {
        if (auto s = files.find(key); s != files.end())
            return s->second;
        else
            return Err(CacheError::NotFound);
    }

    // returns an error if there isnt a value in the cache
    auto operator[](Key &&key) -> Result<T, CacheError> {
        if (auto s = files.find(key); s != files.end())
            return s->second;
        else
            return Err(CacheError::NotFound);
    }

private:
    FileMap files;
};