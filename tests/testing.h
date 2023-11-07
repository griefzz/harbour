#pragma once
#include <cassert>
#include <functional>
#include <iostream>
#include <vector>
#include <string_view>
#include <print>

#define tassert(op, t, u)                                         \
    do {                                                          \
        if (!(op)) {                                              \
            std::print("{} Failed with t={}, u={}\n", #op, t, u); \
            assert(false);                                        \
        }                                                         \
    } while (0);

template<typename T, typename U>
void EXPECT_EQ(T t, U u) {
    tassert(t == u, t, u);
}
template<typename T, typename U>
void EXPECT_NE(T t, U u) {
    tassert(t != u, t, u);
}
template<typename T, typename U>
void EXPECT_GT(T t, U u) {
    tassert(t > u, t, u);
}
template<typename T, typename U>
void EXPECT_GE(T t, U u) {
    tassert(t >= u, t, u);
}
template<typename T, typename U>
void EXPECT_LT(T t, U u) {
    tassert(t < u, t, u);
}
template<typename T, typename U>
void EXPECT_LE(T t, U u) {
    tassert(t <= u, t, u);
}

struct TestCaseInfo {
    std::string_view suite_name;
    std::string_view test_name;
    std::function<void()> test_func;
};

std::vector<TestCaseInfo> t_test_cases;

void RUN_ALL_TESTS() {
    for (auto &test: t_test_cases) {
        std::print("Running {} - {:<{}}:\t", test.suite_name, test.test_name, 15);
        test.test_func();
        std::print("Pass\n");
    }
    std::print("All tests pass\n");
}

#define TEST(suite_name, test_name)                                              \
    void Test_##suite_name##_##test_name();                                      \
    struct TestRegistrar_##suite_name##_##test_name {                            \
        TestRegistrar_##suite_name##_##test_name() {                             \
            t_test_cases.push_back(                                              \
                    {#suite_name, #test_name, Test_##suite_name##_##test_name}); \
        }                                                                        \
    };                                                                           \
    TestRegistrar_##suite_name##_##test_name                                     \
            t_registrar_##suite_name##_##test_name;                              \
    void Test_##suite_name##_##test_name()