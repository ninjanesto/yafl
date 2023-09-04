/**
 * \file
 * \brief       Yet Another Functional Library
 *
 * \project     Critical TechWorks SA
 * \copyright   Critical TechWorks SA
 */
#pragma once

#include <iostream>
#include <memory>
#include <functional>
#include <type_traits>
#include "yafl/Functor.h"
#include "yafl/Applicative.h"
#include "yafl/Monad.h"
#include "yafl/TypeTraits.h"

namespace yafl {

template <typename T>
struct MaybeTraits;

template <template <typename> typename Maybe, typename Inner>
struct MaybeTraits<Maybe<Inner>> {
    using ValueType = Inner;
};
template <template <typename> typename Maybe, typename Inner>
struct MaybeTraits<const Maybe<Inner>> {
    using ValueType = Inner;
};

//Monad Maybe
template <typename T>
class Maybe;

template<>
class Maybe<void> : public Functor<Maybe,void>,
                    public Monad<Maybe,void> {
    friend class Functor<Maybe,void>;
    friend class Monad<Maybe,void>;
private:
    explicit Maybe(bool v) : _value(v) {}

public:
    static Maybe<void> Nothing() {
        return Maybe<void>(false);
    }

    static Maybe<void> Just() {
        return Maybe<void>{true};
    }

    [[nodiscard]] bool hasValue() const { return _value; }

private:
    template <typename Callable>
    decltype(auto) internal_bind(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable>, "Input argument is not invocable");
        using ReturnType = std::invoke_result_t<Callable>;
        if (this->hasValue()) {
            return std::invoke<Callable>(std::forward<Callable>(callable));
        } else {
            return ReturnType::Nothing();
        }
    }

    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable>, "Input argument is not invocable");
        using ReturnType = std::invoke_result_t<Callable>;
        if (this->hasValue()) {
            if constexpr (std::is_void_v<ReturnType>) {
                std::invoke<Callable>(std::forward<Callable>(callable));
                return Maybe<void>::Just();
            } else {
                return Maybe<ReturnType>::Just(std::invoke<Callable>(std::forward<Callable>(callable)));
            }
        } else {
            return Maybe<ReturnType>::Nothing();
        }
    }
private:
    bool _value;
};


template <typename T>
class Maybe : public Functor<Maybe, T>,
              public Applicative<Maybe, T>,
              public Monad<Maybe, T> {
    friend class Functor<Maybe, T>;
    friend class Applicative<Maybe, T>;
    friend class Monad<Maybe, T>;

    static_assert(!std::is_reference_v<T>, "Maybe class cannot store reference to value");

private:
    Maybe() : _value{nullptr}{}
    explicit Maybe(const T& value) :_value{std::make_unique<T>(value)}{}
public:
    static Maybe<T> Nothing() {
        return Maybe<T>();
    }

    static Maybe<T> Just(const T& value) {
        return Maybe<T>(value);
    }

    [[nodiscard]] bool hasValue() const { return !!_value; }

    [[nodiscard]] T value() const {
        if (_value) return *(_value.get());
        throw std::runtime_error("Nothing");
    }

    [[nodiscard]] T valueOr(const T& _v) const {
        return hasValue() ? value() : _v;
    }
private:
    template <typename Callable>
    decltype(auto) internal_bind(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable, T>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<Callable, T>>;
        if (this->hasValue()) {
            return std::invoke<Callable>(std::forward<Callable>(callable), this->value());
        } else {
            return ReturnType::Nothing();
        }
    }

    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable, T>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<Callable, T>>;
        if (this->hasValue()) {
            if constexpr (std::is_void_v<ReturnType>) {
                std::invoke<Callable>(std::forward<Callable>(callable), this->value());
                return Maybe<ReturnType>::Just();
            } else {
                return Maybe<ReturnType>::Just(std::invoke<Callable>(std::forward<Callable>(callable), this->value()));
            }
        } else {
            return Maybe<ReturnType>::Nothing();
        }
    }

    template<typename Head>
    decltype(auto) internal_apply(Maybe<Head>&& arg) const {
        const auto var{std::move(arg)};
        if constexpr (std::is_invocable_v<T, Head>) {
            using ReturnType = std::remove_reference_t<std::invoke_result_t<T, Head>>;
            if (!this->hasValue()) {
                return Maybe<ReturnType>::Nothing();
            } else {
                if (var.hasValue()) {
                    if constexpr (std::is_void_v<ReturnType>) {
                        std::invoke<T>(value(), var.value());
                        return Maybe<ReturnType>::Just();
                    } else {
                        return Maybe<ReturnType>::Just(std::invoke<T>(value(), var.value()));
                    }
                } else {
                    return Maybe<ReturnType>::Nothing();
                }
            }
        } else {
            if (var.hasValue() && this->hasValue()) {
                return Maybe<typename function_traits<T>::PartialApplyFirst>::Just([callable = value(), first = var.value()](auto&& ...args) {
                    return std::apply(callable, std::tuple_cat(std::make_tuple(first), std::make_tuple(args...)));
                });
            } else {
                return Maybe<typename function_traits<T>::PartialApplyFirst>::Nothing();
            }
        }
    }

    template<typename Head>
    decltype(auto) internal_apply(Head&& arg) const {
        if constexpr (std::is_invocable_v<T, Head>) {
            using ReturnType = std::remove_reference_t<std::invoke_result_t<T, Head>>;
            if (!this->hasValue()) {
                return Maybe<ReturnType>::Nothing();
            } else {
                if constexpr (std::is_void_v<ReturnType>) {
                    std::invoke<T>(value(), std::forward<Head>(arg));
                    return Maybe<ReturnType>::Just();
                } else {
                    return Maybe<ReturnType>::Just(std::invoke<T>(value(), std::forward<Head>(arg)));
                }
            }
        } else {
            if (this->hasValue()) {
                return Maybe<typename function_traits<T>::PartialApplyFirst>::Just([callable = value(), first = std::forward<Head>(arg)](auto&& ...args) {
                    return std::apply(callable, std::tuple_cat(std::make_tuple(first), std::make_tuple(args...)));
                });
            } else {
                return Maybe<typename function_traits<T>::PartialApplyFirst>::Nothing();
            }
        }
    }

    template<typename std::enable_if<std::is_invocable_v<T>>* = nullptr>
    decltype(auto) internal_apply() const {
        using ReturnType = std::remove_reference_t<std::invoke_result_t<T>>;
        if (!this->hasValue()) {
            return Maybe<ReturnType>::Nothing();
        } else {
            if constexpr (std::is_void_v<ReturnType>) {
                std::invoke<T>(value());
                return Maybe<ReturnType>::Just();
            } else {
                return Maybe<ReturnType>::Just(std::invoke<T>(value()));
            }
        }
    }

private:
    std::unique_ptr<T> _value;
};

template <typename T>
std::ostream& operator<<(std::ostream& s, const Maybe<T>& m) {
    if (m.hasValue()) {
        return s << "Just: " << m.value();
    } else {
        return s << "Nothing";
    }
}

template <typename = void>
std::ostream& operator<<(std::ostream& s, const Maybe<void>& m) {
    if (m.hasValue()) {
        return s << "Just: void";
    } else {
        return s << "Nothing";
    }
}
template<typename ...T>
Maybe<T...> Nothing() { return Maybe<T...>::Nothing(); }

template<typename ...T>
Maybe<T...> Just(T&& ...args) { return Maybe<T...>::Just(std::forward<T>(args)...); }

template<typename = void>
Maybe<void> Just() { return Maybe<void>::Just(); }

} // namespace yafl
