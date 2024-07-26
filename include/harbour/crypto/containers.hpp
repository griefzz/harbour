///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file containers.hpp
/// @brief Contains the implementation details for Harbours crypto secure container types

#pragma once

#include <limits>
#include <new>
#include <vector>
#include <string>

#include <openssl/crypto.h>

namespace harbour {
    namespace crypto {

        /// @brief Maximum size for our secure heap.
        constexpr std::size_t secure_heap_size = 32 * 1024;

        /// @brief Minimum size for our secure heap
        constexpr std::size_t secure_head_min_size = 32;

        /// @brief Secure allocator using OpenSSL that implements the Allocator trait
        ///        This allocator will initialize a single secure heap of size secure_heap_size
        ///        for the remainder of program execution.
        ///        This allocator should only be used for small private data like encryption keys.
        ///        The allocator should help prevent the case of data leaking, revealing your private keys
        ///        and will zero the memory they hold on destruction.
        ///        Take note that when this allocator is destroyed, the underlying secure heap
        ///        will become unavailable to the running process.
        ///
        /// @warning Do not use this as a generic allocator. The underlying secure heap will become
        ///          unavailable to the running process. If you create and destroy too many SecureAllocators
        ///          you run the risk of exhausting all your available memory. This allocator should only be
        ///          called to store *long running* memory such as encryption keys.
        ///
        /// @tparam T Data type of SecureAllocator
        template<typename T>
        struct SecureAllocator {
            using value_type = T;

            template<class U>
            constexpr SecureAllocator(const SecureAllocator<U> &) noexcept {}

            SecureAllocator() {
                if (!CRYPTO_secure_malloc_initialized())
                    if (!CRYPTO_secure_malloc_init(secure_heap_size, secure_head_min_size))
                        throw std::runtime_error("CRYPTO_secure_malloc_init failed to initialize");
            }

            T *allocate(std::size_t n) {
                if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
                    throw std::bad_alloc();

                auto ptr = static_cast<T *>(OPENSSL_secure_malloc(n * sizeof(T)));
                if (!ptr)
                    throw std::bad_alloc();

                if (!CRYPTO_secure_allocated(ptr))
                    throw std::bad_alloc();

                return ptr;
            }

            void deallocate(T *p, std::size_t) { OPENSSL_secure_free(p); }

            ~SecureAllocator() {
                if (!CRYPTO_secure_malloc_done())
                    log::critical("CRYPTO_secure_malloc_done failed. Secure heap was not released!");
            }
        };

        template<class T, class U>
        bool operator==(const SecureAllocator<T> &, const SecureAllocator<U> &) { return true; }

        template<class T, class U>
        bool operator!=(const SecureAllocator<T> &, const SecureAllocator<U> &) { return false; }

        /// @brief Secure vector for storing small private data securely
        /// @tparam T Data type of secure_vector
        template<typename T>
        using secure_vector = std::vector<T, SecureAllocator<T>>;

        /// @brief Secure string for storing small private data securely
        using secure_string = std::basic_string<char, std::char_traits<char>, SecureAllocator<char>>;

    }// namespace crypto
}// namespace harbour