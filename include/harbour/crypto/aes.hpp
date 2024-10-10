///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file aes.hpp
/// @brief Contains the implementation details for Harbours crypto aes methods

#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <cstdint>
#include <ranges>

#include <openssl/evp.h>

#include "random.hpp"
#include "../ranges.hpp"

namespace harbour::crypto::aes256::detail {

    /// @brief Custom deleter for a EVP_CIPHER_CTX *
    struct CtxDeleter {
        void operator()(EVP_CIPHER_CTX *ptr) const {
            EVP_CIPHER_CTX_free(ptr);
        }
    };

}// namespace harbour::crypto::aes256::detail

namespace harbour::crypto::aes256 {

    using ranges::RandomAccessScalarRange;

    /// @brief Encrypt plaintext using a 32-byte key and a randomly generated
    ///        16-byte iv with AES256-CTR. The iv is prepended to our resulting ciphertext
    /// @param plaintext Plaintext to encrypt
    /// @param key Key used for encryption
    /// @return std::optional<std::string> An encrypted string on success, empty on failure
    auto encrypt(RandomAccessScalarRange auto &&plaintext,
                 RandomAccessScalarRange auto &&key) -> std::optional<std::string> {
        using namespace detail;
        std::unique_ptr<EVP_CIPHER_CTX, CtxDeleter> ctx(EVP_CIPHER_CTX_new());
        if (!ctx) return {};

        const EVP_CIPHER *cipher = EVP_aes_256_ctr();
        const auto size          = EVP_CIPHER_iv_length(cipher);

        auto iv = random::string(size).value_or("");
        if (iv.empty()) return {};

        std::string ciphertext(plaintext.size() + size, 0);
        int len;

        if (!EVP_EncryptInit_ex(ctx.get(), cipher, nullptr,
                                reinterpret_cast<const std::uint8_t *>(key.data()),
                                reinterpret_cast<const std::uint8_t *>(iv.data()))) {
            return {};
        }

        if (!EVP_EncryptUpdate(ctx.get(),
                               reinterpret_cast<std::uint8_t *>(ciphertext.data()),
                               &len,
                               reinterpret_cast<const std::uint8_t *>(plaintext.data()),
                               plaintext.size())) {
            return {};
        }
        int ciphertext_len = len;

        if (!EVP_EncryptFinal_ex(ctx.get(), reinterpret_cast<std::uint8_t *>(ciphertext.data() + len), &len)) {
            return {};
        }
        ciphertext_len += len;

        ciphertext.resize(ciphertext_len);

        return iv + ciphertext;
    }

    /// @brief Decrypt ciphertext using a 32-byte key and 16-byte iv with AES256-CTR
    ///        The iv is expected to be prepended to the ciphertext
    /// @param ciphertext Ciphertext to decrypt
    /// @param key Key used for encryption
    /// @return std::optional<std::string> A decrypted string on success, empty on failure
    auto decrypt(RandomAccessScalarRange auto &&ciphertext,
                 RandomAccessScalarRange auto &&key) -> std::optional<std::string> {
        using namespace detail;
        std::unique_ptr<EVP_CIPHER_CTX, CtxDeleter> ctx(EVP_CIPHER_CTX_new());
        if (!ctx) return {};

        const EVP_CIPHER *cipher = EVP_aes_256_ctr();
        const std::size_t size   = EVP_CIPHER_iv_length(cipher);

        std::string plaintext(ciphertext.size() - size, 0);
        std::string value(ciphertext.begin() + size, ciphertext.end());
        std::string iv(ciphertext.begin(), ciphertext.begin() + size);
        int len;

        if (!EVP_DecryptInit_ex(ctx.get(), cipher, nullptr,
                                reinterpret_cast<const std::uint8_t *>(key.data()),
                                reinterpret_cast<const std::uint8_t *>(iv.data()))) {
            return {};
        }

        if (!EVP_DecryptUpdate(ctx.get(),
                               reinterpret_cast<std::uint8_t *>(plaintext.data()),
                               &len,
                               reinterpret_cast<const std::uint8_t *>(value.data()),
                               value.size())) {
            return {};
        }
        int plaintext_len = len;

        if (!EVP_DecryptFinal_ex(ctx.get(),
                                 reinterpret_cast<std::uint8_t *>(plaintext.data()) + len,
                                 &len)) {
            return {};
        }
        plaintext_len += len;

        plaintext.resize(plaintext_len);

        return plaintext;
    }

}// namespace harbour::crypto::aes256