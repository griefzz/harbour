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

namespace harbour::ranges {
    namespace detail {

        template<template<typename...> class Container>
        struct convert_to_fn {
            template<typename Range>
            auto operator()(Range &&range) const {
                using T = std::ranges::range_value_t<Range>;
                Container<T> container;
                for (auto &&subrange: range) {
                    container.emplace_back(subrange.begin(), subrange.end());
                }
                return container;
            }
        };

        template<template<typename...> class Container>
        inline constexpr convert_to_fn<Container> convert_to{};

        template<typename Adaptor, typename Range>
        concept RangeAdaptorClosure = requires(Adaptor adaptor, Range &&range) {
            { adaptor(std::forward<Range>(range)) };
        };

        template<typename Adaptor>
        struct range_adaptor_closure {
            Adaptor adaptor;

            template<typename Range>
                requires RangeAdaptorClosure<Adaptor, Range>
            friend auto operator|(Range &&range, const range_adaptor_closure &closure) {
                return closure.adaptor(std::forward<Range>(range));
            }
        };

    }// namespace detail

    /// @brief Convert a range to a Container
    template<template<typename...> class Container>
    inline constexpr detail::range_adaptor_closure<detail::convert_to_fn<Container>> to{detail::convert_to<Container>};

    /// @brief Concept for any R that implements a random_access_range and holds a scalar type
    template<typename R>
    concept RandomAccessScalarRange = std::ranges::random_access_range<R> && std::is_scalar_v<std::ranges::range_value_t<R>>;

}// namespace harbour::ranges