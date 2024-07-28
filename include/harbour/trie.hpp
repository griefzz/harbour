///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file trie.hpp
/// @brief This file contains the implementation of a Trie data structure.

#pragma once

#include <optional>
#include <string>
#include <memory>
#include <utility>

#include <ankerl/unordered_dense.h>

#include "http/method.hpp"

namespace harbour {

    /// @class Trie
    /// @brief A template class for a Trie data structure.
    /// @tparam T The type of data stored in the Trie.
    template<typename T>
    class Trie {
        /// @brief Node for each entry in the Trie
        struct Node {
            T data;                                                            ///< Data held on the node
            ankerl::unordered_dense::map<char, std::unique_ptr<Node>> children;///< Child nodes
            std::optional<std::string> path;                                   ///< Optional path key for route
            std::optional<http::Method> method;                                ///< Optional required Method
        };

        /// @brief Result of a Trie match, contains the found node and route key
        struct TrieResult {
            std::optional<Node *> node;         ///< Node for the result containing our Ships and
            std::optional<std::string> path_key;///< Optional path key if it exists
            std::optional<http::Method> method; ///< Optional Method constraint

            /// @brief Get the route information of a TrieResult if it exists
            /// @return std::optional<std::pair<std::string, std::string>> Optional route key and value as a pair
            [[nodiscard]] constexpr auto get_route() const noexcept -> std::optional<std::pair<std::string, std::string>> {
                if (node && node.value()->path && path_key)
                    return std::make_pair(node.value()->path.value(), path_key.value());

                return {};
            }

            explicit operator bool() const { return node.has_value(); }
        };

        Node root;///< Root node of our trie (will always be at path '/')

        /// @brief Cleans the key by ensuring it starts and ends with a '/'.
        /// @param key The key to clean.
        /// @return std::string The cleaned key.
        [[nodiscard]] constexpr auto clean(const std::string_view key) const noexcept -> std::string {
            std::string k(key);
            if (!k.empty() && k[0] != '/') k.insert(k.begin(), '/');
            if (!k.ends_with('/')) k.push_back('/');
            return k;
        }

    public:
        /// @brief Inserts a key-value pair into the Trie.
        /// @param key The key to insert.
        /// @param value The value to insert.
        constexpr auto insert(std::optional<http::Method> method, const std::string_view key, T &&value) noexcept -> void {
            // create a temporary key since we might shift its
            // index forward if it has a Method constraint
            std::string_view _key = key;

            const auto cleaned = clean(_key);
            auto *node         = &root;

            for (std::size_t i = 0; i < cleaned.size(); i++) {
                // Check for wildcard
                if (cleaned[i] == ':') {
                    // +1 to skip ':'
                    // -1 to skip ending path seperator '/'
                    node->path = std::string(cleaned.begin() + i + 1, cleaned.end() - 1);
                    break;
                }

                // Insert empty next node into map
                auto [next, inserted] = node->children.emplace(cleaned[i], nullptr);
                if (inserted) next->second = std::make_unique<Node>();

                // Iterate to next node
                node = next->second.get();
            }

            node->method = method;
            node->data   = std::forward<T>(value);
        }

        /// @brief Matches a key in the Trie and returns the associated Node.
        /// @param key The key to match.
        /// @return An optional containing the value if found, otherwise
        /// std::nullopt.
        [[nodiscard]] constexpr auto match(const std::string_view key) noexcept -> std::optional<TrieResult> {
            const auto cleaned = clean(key);
            auto *node         = &root;


            for (std::size_t i = 0; i < cleaned.size(); i++) {
                // Check for path key
                std::optional<std::string> path_value;
                if (node->path)// do -1 to skip the path seperator '/'
                    path_value = std::string(cleaned.begin() + i, cleaned.end() - 1);

                // If we have a path key or method constraint here return the TrieResult
                if (node->path || node->method)
                    return TrieResult{node, path_value, node->method};

                // Check if we're at the end of our Trie and return empty if so
                const auto next = node->children.find(cleaned[i]);
                if (next == node->children.end()) return {};

                // Iterate to next node
                node = next->second.get();
            }

            return TrieResult{node, std::nullopt, std::nullopt};
        }
    };

}// namespace harbour