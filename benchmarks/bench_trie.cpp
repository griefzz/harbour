#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <memory>

#include <harbour/harbour.hpp>
#include <benchmark/benchmark.h>

using Ships = std::vector<harbour::detail::Ship>;

struct Vec {
    std::vector<std::string> routes;
    std::vector<Ships> ships;

    auto insert(auto &&route, auto &&ship) {
        auto r = std::move(route);
        if (!r.starts_with('/')) r = "/" + r;
        if (!r.ends_with('/')) r += "/";
        routes.emplace_back(r);
        ships.emplace_back(ship);
    }

    auto match(auto &&route) -> std::optional<Ships> {
        if (auto v = std::find(routes.begin(), routes.end(), route); v != routes.end())
            return ships[std::distance(routes.begin(), v)];
        return {};
    }
};

auto make_vec(auto &&routes) -> Vec {
    Vec vec;
    for (const auto &route: routes)
        vec.insert(route, Ships{[] {}});
    return vec;
}

auto make_trie(auto &&routes) -> harbour::Trie<Ships> {
    harbour::Trie<Ships> trie{};
    for (const auto &route: routes)
        trie.insert(route, Ships{[] {}});
    return trie;
}

auto rand_strs(size_t n) -> std::vector<std::string> {
    const std::string m = "abcdefghijklmnopqrstuvwxyz";
    std::srand(std::time(0));
    std::vector<std::string> rstrs;
    for (size_t i = 0; i < n; i++) {
        std::string r;
        for (size_t j = 0; j < rand() % 20 + 1; j++) {
            r += m[rand() % m.size()];
        }
        rstrs.push_back(r);
    }

    return rstrs;
}

static auto rstr0 = rand_strs(10);
static auto rstr1 = rand_strs(100);
static auto rstr2 = rand_strs(1000);
static auto rstr3 = rand_strs(10000);
static auto rstr4 = rand_strs(100000);

static void BM_Trie(benchmark::State &state) {
    std::unique_ptr<harbour::Trie<Ships>> trie;
    if (state.range(0) == 10) trie = std::make_unique<harbour::Trie<Ships>>(make_trie(rstr0));
    if (state.range(0) == 100) trie = std::make_unique<harbour::Trie<Ships>>(make_trie(rstr1));
    if (state.range(0) == 1000) trie = std::make_unique<harbour::Trie<Ships>>(make_trie(rstr2));
    if (state.range(0) == 10000) trie = std::make_unique<harbour::Trie<Ships>>(make_trie(rstr3));
    if (state.range(0) == 100000) trie = std::make_unique<harbour::Trie<Ships>>(make_trie(rstr4));
    for (auto _: state) {
        benchmark::DoNotOptimize(trie->match(std::string("/api/v1/foo/bar/baz/boz")));
    }
}
BENCHMARK(BM_Trie)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_Vec(benchmark::State &state) {
    std::unique_ptr<Vec> vec;
    if (state.range(0) == 10) vec = std::make_unique<Vec>(make_vec(rstr0));
    if (state.range(0) == 100) vec = std::make_unique<Vec>(make_vec(rstr1));
    if (state.range(0) == 1000) vec = std::make_unique<Vec>(make_vec(rstr2));
    if (state.range(0) == 10000) vec = std::make_unique<Vec>(make_vec(rstr3));
    if (state.range(0) == 100000) vec = std::make_unique<Vec>(make_vec(rstr4));
    for (auto _: state) {
        benchmark::DoNotOptimize(vec->match(std::string("/api/v1/foo/bar/baz/boz")));
    }
}
BENCHMARK(BM_Vec)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();