#pragma once
#include <expected>
#include "logger.h"
#include "io.h"
#include "config.h"

enum class CacheError {
    NotFound
};

class Cache {
public:
    Cache() {
        if (auto cache = cache_files(ServerWebPath); cache.has_value()) {
            files = *cache;
        } else {
            Logger::error(std::format("Failed to initialize cache: {}\n", fme_to_string(cache.error())));
            abort();
        }
    }

    using Key = FileMap::key_type;
    using T   = FileMap::mapped_type;

    // returns an error if there isnt a value in the cache
    auto operator[](const Key &key) -> std::expected<T, CacheError> {
        if (auto s = files.find(key); s != files.end())
            return s->second;
        else
            return std::unexpected(CacheError::NotFound);
    }

    // returns an error if there isnt a value in the cache
    auto operator[](Key &&key) -> std::expected<T, CacheError> {
        if (auto s = files.find(key); s != files.end())
            return s->second;
        else
            return std::unexpected(CacheError::NotFound);
    }

private:
    FileMap files;
};

// static cache of our files generated at run-time
static Cache cache;