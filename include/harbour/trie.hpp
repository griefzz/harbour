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

#include <ankerl/unordered_dense.h>

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
        };

        /// @brief Result of a Trie match, contains the found node and route key
        struct TrieResult {
            std::optional<Node *> node;
            std::optional<std::string> path_key;

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
        constexpr auto insert(const std::string_view key, T &&value) noexcept -> void {
            const auto cleaned = clean(key);
            auto *node         = &root;

            for (std::size_t i = 0; i < key.size(); i++) {
                // Check for wildcard
                if (key[i] == ':') {
                    // +1 to skip ':'
                    node->path = std::string(key.begin() + i + 1, key.end());
                    break;
                }

                // Insert empty next node into map
                auto [next, inserted] = node->children.emplace(key[i], nullptr);
                if (inserted) next->second = std::make_unique<Node>();

                // Iterate to next node
                node = next->second.get();
            }

            node->data = std::forward<T>(value);
        }

        /// @brief Matches a key in the Trie and returns the associated Node.
        /// @param key The key to match.
        /// @return An optional containing the value if found, otherwise
        /// std::nullopt.
        [[nodiscard]] constexpr auto match(const std::string_view key) noexcept -> TrieResult {
            const auto cleaned = clean(key);
            auto *node         = &root;


            for (std::size_t i = 0; i < key.size(); i++) {
                // Check for path key
                if (node->path) {
                    auto path_value = std::string(key.begin() + i, key.end());
                    return {node, path_value};
                }

                // Check if we're at the end of our Trie and return empty if so
                const auto next = node->children.find(key[i]);
                if (next == node->children.end()) return {};

                // Iterate to next node
                node = next->second.get();
            }

            return {node, {}};
        }
    };

}// namespace harbour