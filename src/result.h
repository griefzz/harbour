#pragma once
#include <exception>
#include <type_traits>
#include <variant>
#include <string>

using namespace std::string_literals;

template<class E>
class bad_expected_access : public std::exception {
public:
    explicit bad_expected_access(E e) : e(e) {}

    const E &error() const & noexcept { return e; }
    E &error() & noexcept { return e; }
    const E &&error() const && noexcept { return std::move(e); }
    E &&error() && noexcept { return std::move(e); }

    const char *what() const noexcept override {
        return "Tried to return a value when Result contains an error";
    }

private:
    E e;
};

template<class E>
class Err {
public:
    constexpr explicit Err(E &&e) : e(std::move(e)) {}
    constexpr explicit Err(E &e) : e(e) {}

    constexpr const E &error() const & noexcept { return e; }
    constexpr E &error() & noexcept { return e; }
    constexpr const E &&error() const && noexcept { return std::move(e); }
    constexpr E &&error() && noexcept { return std::move(e); }

private:
    E e;
};

template<class T, class E = std::string>
class Result {
    bool err;
    T t;
    E e;

public:
    ///
    constexpr explicit Result(const T &v) : t(v) {}
    constexpr explicit Result(const E &v) : e(v) {}

    template<class U = T>
    constexpr explicit(!std::is_convertible_v<U, T>) Result(U &&v) : t(v), err(false) {}

    template<class G>
    constexpr explicit(!std::is_convertible_v<const G &, E>)
            Result(const Err<G> &e) : e(e.error()), err(true) {}

    template<class G>
    constexpr explicit(!std::is_convertible_v<G, E>)
            Result(Err<G> &&e) : e(e.error()), err(true) {}

    ///
    constexpr bool has_value() const noexcept { return !err; }
    constexpr explicit operator bool() const noexcept { return has_value(); }

    ///
    constexpr T &value() & {
        if (!has_value())
            throw bad_expected_access<std::decay_t<E>>(error());
        return t;
    }
    constexpr const T &value() const & { return value(); }
    constexpr T &&value() && { return std::move(value()); }
    constexpr const T &&value() const && { return std::move(value()); }

    ///
    constexpr const E &error() const & noexcept { return e; }
    constexpr E &error() & noexcept { return e; }
    constexpr const E &&error() const && noexcept { return std::move(e); }
    constexpr E &&error() && noexcept { return std::move(e); }

    ///
    constexpr const T *operator->() const noexcept { return &t; }
    constexpr T *operator->() noexcept { return &t; }
    constexpr const T &operator*() const & noexcept { return t; }
    constexpr T &operator*() & noexcept { return t; }
    constexpr const T &&operator*() const && noexcept { return std::move(t); }
    constexpr T &&operator*() && noexcept { return std::move(t); }

    ///
    template<class U>
    constexpr T value_or(U &&default_value) const & {
        return bool(*this) ? **this
                           : static_cast<T>(std::forward<U>(default_value));
    }
    template<class U>
    constexpr T value_or(U &&default_value) && {
        return bool(*this) ? std::move(**this)
                           : static_cast<T>(std::forward<U>(default_value));
    };
};
