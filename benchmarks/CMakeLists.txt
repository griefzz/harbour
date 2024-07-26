include(FetchContent)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable testing" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Enable testing" FORCE)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON CACHE BOOL "Enable testing" FORCE)

FetchContent_Declare(benchmark
    GIT_REPOSITORY https://github.com/google/benchmark
    GIT_TAG main
)
FetchContent_MakeAvailable(benchmark)

macro(hb_add_benchmark target)
    add_executable(${target} ${target}.cpp)
    target_link_libraries(${target} harbour benchmark::benchmark)
    target_compile_features(${target} PRIVATE cxx_std_20)
endmacro()

hb_add_benchmark(bench_requests)
hb_add_benchmark(bench_trie)