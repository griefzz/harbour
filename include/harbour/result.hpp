/// @file result.hpp
///
/// @brief Header file for creating a Result type
///
/// This file contains the declarations for functions to construct a Result type.
/// This type holds either a successful result or an error for the operation
#pragma once
#include <exception>
#include <type_traits>

using namespace std::string_literals;

/// @brief Exception class for handling bad access on expected results.
/// @tparam E The error type.
template<class E>
class BadExpectedAccess : public std::exception {
public:
    /// @brief Construct a new BadExpectedAccess object.
    /// @param e The error object.
    explicit BadExpectedAccess(E e) : e(std::move(e)) {}

    /// @brief Get the error object
    /// @return The error object.
    [[nodiscard]] auto error() const & noexcept -> const E & { return e; }

    /// @overload
    [[nodiscard]] auto error() & noexcept -> E & { return e; }

    /// @overload
    [[nodiscard]] auto error() const && noexcept -> const E && { return std::move(e); }

    /// @overload
    [[nodiscard]] auto error() && noexcept -> E && { return std::move(e); }

    /// @brief Override the what() function to provide the exception message.
    /// @return const char* The exception message.
    [[nodiscard]] auto what() const noexcept -> const char * override {
        return "Tried to access the wrong type of a Result";
    }

private:
    E e;///< The error object.
};

/// @brief Wrapper class for error types.
///
/// @tparam E The error type.
template<class E>
class Err {
public:
    /// @brief Construct a new Err object
    /// @param e The error object.
    constexpr explicit Err(E &&e) : e(std::move(e)) {}

    /// @overload
    constexpr explicit Err(E &e) : e(std::forward<E>(e)) {}

    /// @brief Get the error object
    /// @return The error object.
    [[nodiscard]] constexpr auto error() const & noexcept -> const E & { return e; }

    /// @overload
    [[nodiscard]] constexpr auto error() & noexcept -> E & { return e; }

    /// @overload
    [[nodiscard]] constexpr auto error() const && noexcept -> const E && { return std::move(e); }

    /// @overload
    [[nodiscard]] constexpr auto error() && noexcept -> E && { return std::move(e); }

private:
    E &&e;///< The error object.
};

/// @brief Class representing a result which can either be a value or an error.
/// @tparam T The value type.
/// @tparam E The error type, defaults to std::string.
template<class T, class E = std::string>
class Result {
    bool err;///< Flag indicating if the result is an error.
    union {
        T t;///< The value object.
        E e;///< The error object.
    };

public:
    /// @brief Construct a new Result object from a value.
    /// @param v The value object.
    constexpr explicit Result(const T &v) : t(v), err(false) {}

    /// @brief Construct a new Result object from an error.
    /// @param v The error object.
    constexpr explicit Result(const E &v) : e(v), err(true) {}

    /// @overload
    template<class U = T>
    constexpr explicit(!std::is_convertible_v<U, T>) Result(const U &v)
        : t(std::move(v)), err(false) {}

    /// @overload
    template<class G>
    constexpr explicit(!std::is_convertible_v<const G &, E>) Result(const Err<G> &e)
        : e(e.error()), err(true) {}

    /// @overload
    template<class G>
    constexpr explicit(!std::is_convertible_v<G, E>) Result(Err<G> &e)
        : e(e.error()), err(true) {}

    /// @brief Destroy the Result object.
    ~Result() { (has_value() ? t.~T() : e.~E()); }

    /// @brief Check if the result has a value.
    /// @return true If the result has a value.
    /// @return false If the result is an error.
    [[nodiscard]] constexpr auto has_value() const noexcept -> bool { return !err; }

    /// @brief Convert to bool to check if the result has a value.
    /// @return true If the result has a value.
    /// @return false If the result is an error.
    constexpr explicit operator bool() const noexcept { return has_value(); }

    /// @brief Get the value object.
    /// @return The value object.
    constexpr auto value() & -> T & {
        if (!has_value())
            throw BadExpectedAccess<std::decay_t<E>>(error());
        return t;
    }

    /// @overload
    [[nodiscard]] constexpr auto value() const & -> const T & {
        if (!has_value())
            throw BadExpectedAccess<std::decay_t<E>>(error());
        return t;
    }

    /// @overload
    constexpr auto value() && -> T && {
        if (!has_value())
            throw BadExpectedAccess<std::decay_t<E>>(error());
        return std::move(t);
    }

    /// @overload
    [[nodiscard]] constexpr auto value() const && -> const T && {
        if (!has_value())
            throw BadExpectedAccess<std::decay_t<E>>(error());
        return std::move(t);
    }

    /// @brief Get the error object.
    /// @return The error object.
    constexpr auto error() & noexcept -> E & {
        return e;
    }

    /// @overload
    [[nodiscard]] constexpr auto error() const & noexcept -> const E & {
        return e;
    }

    /// @overload
    constexpr auto error() && noexcept -> E && {
        return std::move(e);
    }

    /// @overload
    [[nodiscard]] constexpr auto error() const && noexcept -> const E && {
        return std::move(e);
    }

    /// @brief Access the value object through pointer semantics.
    /// @return Pointer to the value object.
    constexpr auto operator->() const noexcept -> const T * { return &t; }

    /// @overload
    constexpr auto operator->() noexcept -> T * { return &t; }

    /// @brief Access the value object through reference semantics.
    /// @return Reference to the value object.
    constexpr auto operator*() const & noexcept -> const T & { return t; }

    /// @overload
    constexpr auto operator*() & noexcept -> T & { return t; }

    /// @overload
    constexpr auto operator*() const && noexcept -> const T && { return std::move(t); }

    /// @overload
    constexpr auto operator*() && noexcept -> T && { return std::move(t); }

    /// @brief Get the value or a default if the result is an error.
    /// @tparam U The type of the default value.
    /// @param default_value The default value to return if the result is an error.
    /// @return T The value or the default.
    template<class U>
    [[nodiscard]] constexpr auto value_or(U &&default_value) const & -> T {
        return has_value() ? value() : static_cast<T>(std::forward<U>(default_value));
    }

    /// @overload
    template<class U>
    constexpr auto value_or(U &&default_value) && -> T {
        return has_value() ? std::move(value()) : static_cast<T>(std::forward<U>(default_value));
    };
};