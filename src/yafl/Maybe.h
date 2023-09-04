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
#include <optional>
#include <functional>
#include <type_traits>
#include "yafl/Functor.h"
#include "yafl/Applicative.h"
#include "yafl/Monad.h"
#include "yafl/TypeTraits.h"

namespace yafl {

/**
 * @ingroup Maybe
 * Maybe class. This class implements the Maybe type by realizing concepts from functional programming
 * such as Functor, Applicative and Monad. The implementation for maybe class was based on Haskell Maybe
 * type as defined in https://hackage.haskell.org/package/base-4.18.0.0/docs/Data-Maybe.html
 */
template <typename>
class Maybe;
namespace type{
namespace details {

/**
 * @ingroup Details
 * Maybe monad traits specialization that enable getting the inner type
 * @tparam Inner Inner type
*/
template<typename Inner>
struct DomainDetailsImpl<Maybe<Inner>> {
    /// Functor Base type
    using FBaseType = core::Functor<Maybe, Inner>;
    /// Applicative Base type
    using ABaseType = core::Applicative<Maybe, Inner>;
    /// Monad Base type
    using MBaseType = core::Monad<Maybe, Inner>;
    /// Value Type
    using ValueType = Inner;
    /// Derived type
    using DerivedType = Maybe<Inner>;
    ///boolean flag that states whether type T is a Functor or not
    static constexpr bool hasFunctorBase = std::is_base_of_v<FBaseType, DerivedType>;
    ///boolean flag that states whether type T is an Applicative or not
    static constexpr bool hasApplicativeBase = std::is_base_of_v<ABaseType, DerivedType>;
    ///boolean flag that states whether type T is a Monad or not
    static constexpr bool hasMonadicBase = std::is_base_of_v<MBaseType, DerivedType>;
    ///Callback responsible for handling errors
    static constexpr auto handleError = [](auto&& ...) {
        return DerivedType::Nothing();
    };
};
} // namespace details
} // namespace type

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
     * Copy constructor
     * @param other argument to be copied
     */
    Maybe(const Maybe<void>& other) = default;

    /**
     * Move constructor
     * @param other argument to be moved
     */
    Maybe(Maybe<void>&& other) noexcept = default;

    /**
     * Assignment operator
     * @param other argument to be copied
     * @return Maybe with the value copied
     */
    Maybe<void>& operator=(const Maybe& other) = default;

    /**
     * Move operator
     * @param other argument to be moved
     * @return Maybe with the value moved
     */
    Maybe<void>& operator=(Maybe<void>&& other) noexcept = default;

    /**
     * Comparison operator overload
     * @param other instance of maybe to compare to
     * @return true if objects are equal and false otherwise
     */
    bool operator==(const Maybe<void>& other) const noexcept {
        return _value == other._value;
    }

    /**
     * Logical not operator
     * @return false if maybe has value and true otherwise
     */
    bool operator!() const {
        return !_value;
    }

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
        static_assert(std::is_invocable_v<std::decay_t<Callable>>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<Callable>>>;
        if (hasValue()) {
            return std::invoke<Callable>(std::forward<Callable>(callable));
        } else {
            return ReturnType::Nothing();
        }
    }

    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<std::decay_t<Callable>>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<Callable>>>;
        if (hasValue()) {
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
    Maybe() : _value{}{}
    explicit Maybe(const T& value) : _value{value}{}
public:
    /**
     * Copy constructor
     * @param maybe argument to be copied
     */
    Maybe(const Maybe<T>& maybe) = default;

    /**
     * Move constructor
     * @param maybe arguent to be moved
     */
    Maybe(Maybe<T>&& maybe) noexcept = default;

    /**
     * Assignment operator
     * @param other argument to be copied
     * @return Maybe with the value copied
     */
    Maybe<T>& operator=(const Maybe<T>& other) = default;

    /**
     * Move operator
     * @param other argument to be moved
     * @return Maybe with the value moved
     */
    Maybe<T>& operator=(Maybe<T>&& other) noexcept = default;

    /**
     * Comparison operator overload
     * @param other instance of maybe to compare to
     * @return true if objects are equal and false otherwise
     */
    bool operator==(const Maybe<T>& other) const noexcept {
        return  _value == other._value;
    }

    /**
     * Logical not operator
     * @return false if maybe has value and true otherwise
     */
    bool operator!() const {
        return !hasValue();
    }

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
    [[nodiscard]] bool hasValue() const { return _value.has_value(); }

    /**
     * Extracts the wrapped value from the Maybe
     * @return the value wrapped
     * @throws std::runtime_error when maybe contains nothing
     */
    [[nodiscard]] T value() const {
        if (_value) return _value.value();
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
        static_assert(std::is_invocable_v<std::decay_t<Callable>, std::decay_t<T>>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<Callable>, std::decay_t<T>>>;
        if (hasValue()) {
            return std::invoke<Callable>(std::forward<Callable>(callable), value());
        } else {
            return ReturnType::Nothing();
        }
    }

    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<std::decay_t<Callable>, std::decay_t<T>>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<Callable>, std::decay_t<T>>>;
        if (hasValue()) {
            if constexpr (std::is_void_v<ReturnType>) {
                std::invoke<Callable>(std::forward<Callable>(callable), value());
                return Maybe<ReturnType>::Just();
            } else {
                return Maybe<ReturnType>::Just(std::invoke<Callable>(std::forward<Callable>(callable), value()));
            }
        } else {
            return Maybe<ReturnType>::Nothing();
        }
    }

    template <typename Arg>
    decltype(auto) internal_apply(Arg&& arg) const {
        static_assert(!std::is_invocable_v<std::decay_t<T>>, "Function that takes 0 arguments cannot be called with arguments");
        if constexpr (type::DomainTypeInfo<Arg>::hasMonadicBase) {
            if (arg.hasValue()) {
                return internal_apply_non_monad(arg.value());
            } else {
                using ArgInnerType = std::remove_reference_t<typename type::DomainTypeInfo<Arg>::ValueType>;
                if constexpr (std::is_invocable_v<std::decay_t<T>, std::decay_t<ArgInnerType>>) {
                    using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<T>, std::decay_t<ArgInnerType>>>;
                    return Maybe<ReturnType>::Nothing();
                } else {
                    using ReturnType = std::remove_reference_t<typename function::Info<T>::PartialApplyFirst>;
                    return Maybe<ReturnType>::Nothing();
                }
            }
        } else {
            return internal_apply_non_monad(std::forward<Arg>(arg));
        }
    }

    template<typename Arg>
    decltype(auto) internal_apply_non_monad(Arg&& arg) const {
        static_assert(!std::is_invocable_v<std::decay_t<T>>, "Function that takes 0 arguments cannot be called with arguments");
        if constexpr (std::is_invocable_v<std::decay_t<T>, std::decay_t<Arg>>) {
            using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<T>, std::decay_t<Arg>>>;
            if (!hasValue()) {
                return Maybe<ReturnType>::Nothing();
            } else {
                if constexpr (std::is_void_v<ReturnType>) {
                    std::invoke<T>(value(), std::move(arg));
                    return Maybe<ReturnType>::Just();
                } else {
                    return Maybe<ReturnType>::Just(std::invoke<T>(value(), std::move(arg)));
                }
            }
        } else {
            using PartialFunctionType = std::remove_reference_t<typename function::Info<T>::PartialApplyFirst>;

            if (hasValue()) {
                return Maybe<PartialFunctionType>::Just(
                        [callable = value(), first = std::forward<Arg>(arg)](auto&& ...args) mutable {
                            return callable(std::move(first), std::forward<decltype(args)>(args)...);
                        });
            } else {
                return Maybe<PartialFunctionType>::Nothing();
            }
        }
    }

    decltype(auto) internal_apply() const {
        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<T>>>;
        if (!hasValue()) {
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
    std::optional<T> _value;
};

namespace maybe {

/**
 * @ingroup Maybe
 * Function that helps build a Maybe with Nothing
 * @tparam ValueType Maybe inner type
 * @return maybe with nothing
 */
template<typename ValueType>
Maybe<std::remove_reference_t<ValueType>> Nothing() {
    return Maybe<std::remove_reference_t<ValueType>>::Nothing();
}

/**
 * @ingroup Maybe
 * Function that helps build a Maybe with a void "value"
 * @tparam ValueType inner type defaulted to void
 * @return maybe with void value
 */
template<typename T = void>
Maybe<void> Just() { return Maybe<void>::Just(); }

/**
 * @ingroup Maybe
 * Function that helps build a Maybe with a value
 * @tparam ValueType Maybe inner type
 * @param args Argument to be wrapped
 * @return maybe with the value wrapped
 */
template<typename ValueType>
Maybe<std::remove_reference_t<ValueType>> Just(ValueType&& args) {
    return Maybe<std::remove_reference_t<ValueType>>::Just(std::forward<ValueType>(args));
}

/**
 * @ingroup Maybe
 * Lift given callable into the given abstract monadic type
 * @tparam Callable Callable type to lift
 * @param callable Callable to lift
 * @return callable lifted into the given abstract monadic type
 */
template<typename Callable>
decltype(auto) lift(Callable &&callable) {
    if constexpr (std::is_invocable_v<std::decay_t<Callable>>) {
        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<Callable>>>;
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
        using ReturnType = typename function::Info<Callable>::ReturnType;
        using ReturnFunctionType = typename function::Info<Callable>::template LiftedSignature<Maybe>;

        const ReturnFunctionType function = [callable = std::forward<Callable>(callable)](auto&& ...args) -> Maybe<ReturnType> {
            if (all_true([](auto&& v) { return v.hasValue(); }, args...)) {
                const auto tp = tuple::map_append([](auto&& arg) { return arg.value(); }, std::make_tuple(), args...);

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

        return function;
    }
}

} // namespace maybe
} // namespace yafl
