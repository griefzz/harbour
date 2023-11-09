#pragma once
#include <exception>
#include <type_traits>
#include <variant>

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

template<class T, class E>
class Result {
    std::variant<T, E> data;

public:
    ///
    constexpr explicit Result(const T &v) : data(v) {}
    constexpr explicit Result(const E &v) : data(v) {}

    template<class U = T>
    constexpr explicit(!std::is_convertible_v<U, T>) Result(U &&v) : data(v) {}

    template<class G>
    constexpr explicit(!std::is_convertible_v<const G &, E>)
            Result(const Err<G> &e) : data(e.error()) {}

    template<class G>
    constexpr explicit(!std::is_convertible_v<G, E>)
            Result(Err<G> &&e) : data(e.error()) {}

    ///
    constexpr bool has_value() const noexcept { return std::holds_alternative<T>(data); }
    constexpr explicit operator bool() const noexcept { return has_value(); }

    ///
    constexpr T &value() & {
        if (!has_value())
            throw bad_expected_access<std::decay_t<E>>(error());
        return std::get<T>(data);
    }
    constexpr const T &value() const & { return value(); }
    constexpr T &&value() && { return std::move(value()); }
    constexpr const T &&value() const && { return std::move(value()); }

    ///
    constexpr const E &error() const & noexcept { return std::get<E>(data); }
    constexpr E &error() & noexcept { return std::get<E>(data); }
    constexpr const E &&error() const && noexcept { return std::move(std::get<E>(data)); }
    constexpr E &&error() && noexcept { return std::move(std::get<E>(data)); }

    ///
    constexpr const T *operator->() const noexcept { return &std::get<T>(data); }
    constexpr T *operator->() noexcept { return &std::get<T>(data); }
    constexpr const T &operator*() const & noexcept { return std::get<T>(data); }
    constexpr T &operator*() & noexcept { return std::get<T>(data); }
    constexpr const T &&operator*() const && noexcept { return std::move(std::get<T>(data)); }
    constexpr T &&operator*() && noexcept { return std::move(std::get<T>(data)); }

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
