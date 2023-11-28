#pragma once
#include <optional>
#include <string>
#include <ranges>
#include <unordered_map>
#include <filesystem>
#include <functional>
#include "server.hpp"

using RouteMap = std::unordered_map<std::string, std::string>;

struct Server;
using Middleware = std::function<void(Server &ctx, const Request &, Response &)>;
using Handler    = std::function<Response(Server &ctx, const Request &)>;

struct Route {
    explicit Route(const std::string &path, Handler &&handler)
        : path(path), handler(handler) { make_path_table(); }

    auto parse(const std::string &reqPath) -> std::optional<RouteMap> {
        if (table.empty())
            return {};

        RouteMap map;
        if (reqPath.find(root) != std::string::npos) {
            auto req = reqPath.substr(root.size(), reqPath.size() - root.size());
            std::vector<std::string> tokens;
            for (const auto &&part: std::views::split(req, '/')) {
                // the last token is always "" for some reason here...
                // i *think* we want to even keep empty tokens in our api path... not sure
                auto s = std::string(std::ranges::begin(part), std::ranges::end(part));
                if (s != "") tokens.emplace_back(s);
            }

            if (tokens.size() != table.size())
                return {};

            for (std::size_t i = 0; i < table.size(); i++) {
                map[table[i]] = tokens[i];
            }
        }

        return map;
    }

    constexpr auto make_path_table() -> void {
        auto start = path.find("<");
        if (start != std::string::npos) {
            root = path.substr(0, start);
        } else {
            root = path;
        }
        while (start != std::string::npos) {
            auto end = path.find(">", start);
            auto var = path.substr(start + 1, end - start - 1);
            table.push_back(std::move(var));
            start = path.find("<", end);
        }
    }

    std::string root;
    std::string path;
    Handler handler;
    std::vector<std::string> table;
};

template<typename T>
concept RouteConcept = std::is_same_v<T, Route>;