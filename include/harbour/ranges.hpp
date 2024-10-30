///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file ranges.hpp
/// @brief Contains the implementation details for the harbours ranges utility functions

#pragma once

#include <ranges>
#include <type_traits>
#include <concepts>
#include <algorithm>

namespace harbour::ranges {
    namespace detail {
        // Concept to check if a type can be constructed from a range
        template<typename Container, typename Range>
        concept ConstructibleFromRange = requires(Range &&range) {
            Container(std::ranges::begin(range), std::ranges::end(range));
        };

        template<template<typename...> class Container>
        struct convert_to_fn {
            template<std::ranges::input_range Range>
                requires std::movable<std::ranges::range_value_t<Range>>
            [[nodiscard]] constexpr auto operator()(Range &&range) const {
                using value_type     = std::ranges::range_value_t<Range>;
                using container_type = Container<value_type>;

                if constexpr (ConstructibleFromRange<container_type, Range>) {
                    return container_type(std::ranges::begin(range), std::ranges::end(range));
                } else {
                    container_type container;
                    if constexpr (requires { container.reserve(std::size_t{}); }) {
                        if constexpr (std::ranges::sized_range<Range>) {
                            container.reserve(std::ranges::size(range));
                        }
                    }
                    std::ranges::copy(range, std::back_inserter(container));
                    return container;
                }
            }
        };

        template<template<typename...> class Container>
        inline constexpr convert_to_fn<Container> convert_to{};

        template<typename Adaptor, typename Range>
        concept RangeAdaptorClosure = requires(Adaptor adaptor, Range &&range) {
            { adaptor(std::forward<Range>(range)) } -> std::ranges::range;
        };

        template<typename Adaptor>
        struct range_adaptor_closure {
            [[no_unique_address]] Adaptor adaptor;

            template<std::ranges::range Range>
                requires RangeAdaptorClosure<Adaptor, Range>
            [[nodiscard]] constexpr friend auto operator|(Range &&range, const range_adaptor_closure &closure) {
                return closure.adaptor(std::forward<Range>(range));
            }
        };

    }// namespace detail

    /// @brief Convert a range to a Container
    template<template<typename...> class Container>
    inline constexpr detail::range_adaptor_closure<detail::convert_to_fn<Container>> to{detail::convert_to<Container>};

    /// @brief Concept for any R that implements a random_access_range and holds a scalar type
    template<typename R>
    concept RandomAccessScalarRange = std::ranges::random_access_range<R> &&
                                      std::is_scalar_v<std::ranges::range_value_t<R>> &&
                                      std::ranges::sized_range<R>;

    /// @brief Concept to ensure range contains char or unsigned char
    template<typename T>
    concept RandomAccessCharRange = ranges::RandomAccessScalarRange<T> &&
                                    (std::same_as<std::ranges::range_value_t<T>, char> ||
                                     std::same_as<std::ranges::range_value_t<T>, unsigned char>);

}// namespace harbour::ranges