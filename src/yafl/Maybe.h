/**
 * \brief       Yet Another Functional Library
 *
 * \copyright   2023, Ernesto Festas.
 *              Distributed under MIT license (See accompanying LICENSE file)
 * \defgroup Maybe Maybe monad
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

namespace maybe {
/**
 * @ingroup Maybe
 * Maybe monad traits
 */
template<typename>
struct Details;

/**
 * @ingroup Maybe
 * Maybe monad traits specialization that enable getting the inner type
 * @tparam MaybeT Maybe monad type
 * @tparam Inner Inner type
*/
template<template<typename> typename MaybeT, typename Inner>
struct Details<MaybeT<Inner>> {
    using ValueType = Inner;
};

/**
 * @ingroup Maybe
 * Maybe monad traits specialization for const types that enable getting the inner type
 * @tparam Maybe Maybe monad type
 * @tparam Inner Inner type
 */
template<template<typename> typename MaybeT, typename Inner>
struct Details<const MaybeT<Inner>> {
    using ValueType = Inner;
};

} // namespace maybe

/**
 * @ingroup Maybe
 * Maybe class. This class implements the Maybe type by realizing concepts from functional programming
 * such as Functor, Applicative and Monad. The implementation for maybe class was based on Haskell Maybe
 * type as defined in https://hackage.haskell.org/package/base-4.18.0.0/docs/Data-Maybe.html
 */
template <typename>
class Maybe;

/**
 * @ingroup Maybe
 * Specialization of the Maybe class for void type
 */
template<>
class Maybe<void> : public core::Functor<Maybe,void>,
                    public core::Monad<Maybe,void> {
    friend class core::Functor<Maybe,void>;
    friend class core::Monad<Maybe,void>;
private:
    explicit Maybe(bool v) : _value(v) {}

public:
    /**
     * Constructs a Maybe type that has Nothing
     * @return maybe nothing
     */
    static Maybe<void> Nothing() {
        return Maybe<void>(false);
    }

    /**
     * Constructs a Maybe type with a "valid" value.
     * Since Maybe is defined with void, this method doesn't
     * receive any argument
     * @return maybe with "void" value
     */
    static Maybe<void> Just() {
        return Maybe<void>{true};
    }

    /**
     * Returns whether maybe has nothing or a valid value
     * @return true if valid and false otherwise
     */
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

/**
 * @ingroup Maybe
 * Generalization of the Maybe class for any type other than void
 */
template <typename T>
class Maybe : public core::Functor<Maybe, T>,
              public core::Applicative<Maybe, T>,
              public core::Monad<Maybe, T> {
    friend class core::Functor<Maybe, T>;
    friend class core::Applicative<Maybe, T>;
    friend class core::Monad<Maybe, T>;

    static_assert(!std::is_reference_v<T>, "Maybe class cannot store reference to value");

private:
    Maybe() : _value{nullptr}{}
    explicit Maybe(const T& value) :_value{std::make_unique<T>(value)}{}
public:
    /**
     * Constructs a Maybe type that has Nothing
     * @return maybe nothing
     */
    static Maybe<T> Nothing() {
        return Maybe<T>();
    }

    /**
     * Constructs a Maybe type with a "valid" value.
     * Since Maybe is defined with void, this method doesn't
     * receive any argument
     * @param value
     * @return maybe with "void" value
     */
    static Maybe<T> Just(const T& value) {
        return Maybe<T>(value);
    }

    /**
     * Checks whether maybe has nothing or a valid value
     * @return true if valid and false otherwise
     */
    [[nodiscard]] bool hasValue() const { return !!_value; }

    /**
     * Extracts the wrapped value from the Maybe
     * @return the value wrapped
     * @throws std::runtime_error when maybe contains nothing
     */
    [[nodiscard]] T value() const {
        if (_value) return *(_value.get());
        throw std::runtime_error("Nothing");
    }

    /**
     * Extracts the wrapped value from the Maybe if exists.
     * If Maybe contains nothing then returns provided default value
     * @param arg default value to return if Maybe contains nothing
     * @return the wrapped value if exists or the default otherwise
     */
    [[nodiscard]] T valueOr(const T& arg) const {
        return hasValue() ? value() : arg;
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
                return Maybe<typename FunctionTraits<T>::PartialApplyFirst>::Just([callable = value(), first = var.value()](auto&& ...args) {
                    return std::apply(callable, std::tuple_cat(std::make_tuple(first), std::make_tuple(args...)));
                });
            } else {
                return Maybe<typename FunctionTraits<T>::PartialApplyFirst>::Nothing();
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
                return Maybe<typename FunctionTraits<T>::PartialApplyFirst>::Just([callable = value(), first = std::forward<Head>(arg)](auto&& ...args) {
                    return std::apply(callable, std::tuple_cat(std::make_tuple(first), std::make_tuple(args...)));
                });
            } else {
                return Maybe<typename FunctionTraits<T>::PartialApplyFirst>::Nothing();
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

namespace maybe {

/**
 * @ingroup Maybe
 * Function that helps build a Maybe with Nothing
 * @tparam T Maybe inner type
 * @return maybe with nothing
 */
template<typename ...T>
Maybe<T...> Nothing() { return Maybe<T...>::Nothing(); }

/**
 * @ingroup Maybe
 * Function that helps build a Maybe with a value
 * @tparam T Maybe inner type
 * @param args Argument to be wrapped
 * @return maybe with the value wrapped
 */
template<typename ...T>
Maybe<T...> Just(T &&...args) { return Maybe<T...>::Just(std::forward<T>(args)...); }

/**
 * @ingroup Maybe
 * Function that helps build a Maybe with a void "value"
 * @tparam T Maybe inner type
 * @return maybe with void value
 */
template<typename = void>
Maybe<void> Just() { return Maybe<void>::Just(); }

/**
 * @ingroup Maybe
 * Lift given callable into the given abstract monadic type
 * @tparam MonadType Abstract type to lift function to
 * @tparam Callable Callable type to lift
 * @param callable Callable to lift
 * @return callable lifted into the given abstract monadic type
 */
template<typename Callable>
decltype(auto) lift(Callable &&callable) {
    if constexpr (std::is_invocable_v<Callable>) {
        using ReturnType = std::remove_reference_t<std::invoke_result_t<Callable>>;
        if constexpr (std::is_void_v<ReturnType>) {
            return [callable = std::forward<Callable>(callable)]() {
                callable();
                return Maybe<ReturnType>::Just();
            };
        } else {
            return [callable = std::forward<Callable>(callable)]() {
                return Maybe<ReturnType>::Just(callable());
            };
        }
    } else {
        using ReturnType = typename yafl::FunctionTraits<Callable>::ReturnType;

        return [callable = std::forward<Callable>(callable)](auto ...args) -> Maybe<ReturnType> {
            if (detail::all_true([](auto &&v) { return v.hasValue(); }, args...)) {
                const auto tp = detail::map_tuple_append([](auto &&arg) { return arg.value(); }, std::make_tuple(),
                                                         args...);

                if constexpr (std::is_void_v<ReturnType>) {
                    std::apply(callable, tp);
                    return Maybe<ReturnType>::Just();
                } else {
                    return Maybe<ReturnType>::Just(std::apply(callable, tp));
                }
            } else {
                return Maybe<ReturnType>::Nothing();
            }
        };
    }
}

} // namespace maybe
} // namespace yafl
