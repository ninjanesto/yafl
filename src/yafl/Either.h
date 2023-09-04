/**
 * \brief       Yet Another Functional Library
 *
 * \copyright   2023, Ernesto Festas.
 *              Distributed under MIT license (See accompanying LICENSE file)
 * \defgroup Either Either monad
 */
#pragma once

#include <iostream>
#include <memory>
#include <functional>
#include <variant>
#include "yafl/Functor.h"
#include "yafl/Monad.h"
#include "yafl/Applicative.h"

namespace yafl {

/**
 * @ingroup Either
 * Either class. This class implements the Either type by realizing concepts from functional programming
 * such as Functor, Applicative and Monad. The implementation for Either class was based on Haskell Either
 * type as defined in https://hackage.haskell.org/package/base-4.18.0.0/docs/Data-Either.html
 */
template<typename Error, typename Value>
class Either;

namespace {

/**
 * @ingroup Type
 * Either monad traits specialization that enable getting the inner error and value types
 * @tparam InnerError Error type
 * @tparam InnerValue Value type
 */
template<typename InnerError, typename InnerValue>
struct DetailsImpl<Either < InnerError, InnerValue>> {
    /// Functor Base type
    using FBaseType = core::Functor<Either, InnerError, InnerValue>;
    /// Applicative Base type
    using ABaseType = core::Applicative<Either, InnerError, InnerValue>;
    /// Monad Base type
    using MBaseType = core::Monad<Either, InnerError, InnerValue>;
    /// Value Type
    using ValueType = InnerValue;
    /// Error Type
    using ErrorType = InnerError;
    /// Derived type
    using DerivedType = Either<InnerError, InnerValue>;
    ///boolean flag that states whether type T is a Functor or not
    static constexpr bool hasFunctorBase = std::is_base_of_v<FBaseType, DerivedType>;
    ///boolean flag that states whether type T is an Applicative or not
    static constexpr bool hasApplicativeBase = std::is_base_of_v<ABaseType, DerivedType>;
    ///boolean flag that states whether type T is a Monad or not
    static constexpr bool hasMonadicBase = std::is_base_of_v<MBaseType, DerivedType>;
    ///Callback responsible for handling errors
    static constexpr auto handleError = []([[maybe_unused]] auto &&...args) {
        static_assert(std::is_same_v<typename type::Details<decltype(args)...>::ErrorType, ErrorType>, "Error types should match");
        if constexpr (std::is_void_v<ErrorType>) {
            return DerivedType::Error();
        } else {
            return DerivedType::Error(args.error()...);
        }
    };
};

}

namespace type {

/**
 * Helper struct that enables fixing error type and
 * exposes template alias for value type
 * @tparam ErrorType
 */
template <typename ErrorType>
struct FixedErrorType {
    template <typename ValueType>
    using Type = Either<ErrorType, ValueType>;
};
} // namespace type

/**
 * @ingroup Either
 * Specialization of the Either class for void error and void value types
 */
template<>
class Either<void, void> : public core::Functor<Either, void, void>
                         , public core::Monad<Either, void, void> {
    friend class core::Functor<Either, void, void>;
    friend class core::Monad<Either, void, void>;

private:
    explicit Either(bool v) : _value(v) {}

public:
    /**
     * Copy constructor
     * @param other argument to be copied
     */
    Either(const Either<void, void>& other) = default;

    /**
     * Move constructor
     * @param other argument to be moved
     */
    Either(Either<void, void>&& other) noexcept = default;

    /**
     * Assignment operator
     * @param other argument to be copied
     * @return Either with the value copied
     */
    Either<void, void>& operator=(const Either<void, void>& other) = default;

    /**
     * Move operator
     * @param other argument to be moved
     * @return Either with the value moved
     */
    Either<void, void>& operator=(Either<void, void>&& other) noexcept = default;

    /**
     * Comparison operator overload
     * @param other instance of either to compare to
     * @return true if objects are equal and false otherwise
     */
    bool operator==(const Either<void, void>& other) const noexcept {
        return _value == other._value;
    }

    /**
     * Logical not operator
     * @return false if either has value and true otherwise
     */
    bool operator!() const {
        return !_value;
    }

    /**
     * Constructs an Either type that is an Error
     * @return Either with error defined
     */
    static Either<void, void> Error() {
        return Either<void, void>(false);
    }

    /**
     * Constructs an Either type that is an Ok
     * @return Either with value defined
     */
    static Either<void, void> Ok() {
        return Either<void, void>{true};
    }

    /**
     * Returns whether Either is an Error or a Value
     * @return true if error and false otherwise
     */
    [[nodiscard]] bool isError() const { return !_value; }

    /**
     * Returns whether Either is an Error or a Value
     * @return true if value and false otherwise
     */
    [[nodiscard]] bool isOk() const { return _value; }

private:
    template <typename Callable>
    decltype(auto) internal_bind(Callable&& callable) const {
        static_assert(std::is_invocable_v<std::decay_t<Callable>>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<Callable>>>;
        using InnerTypeError = typename type::Details<ReturnType>::ErrorType;
        static_assert(std::is_same_v<void, InnerTypeError>, "Error type does not match");
        using InnerTypeOK = typename type::Details<ReturnType>::ValueType;
        if (isOk()) {
            return std::invoke<Callable>(std::forward<Callable>(callable));
        } else {
            return Either<InnerTypeError, InnerTypeOK>::Error();
        }
    }

    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<std::decay_t<Callable>>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<Callable>>>;
        if (isOk()) {
            if constexpr (std::is_void_v<ReturnType>) {
                std::invoke<Callable>(std::forward<Callable>(callable));
                return Either<void, ReturnType>::Ok();
            } else {
                return Either<void, ReturnType>::Ok(std::invoke<Callable>(std::forward<Callable>(callable)));
            }
        } else {
            return Either<void, ReturnType>::Error();
        }
    }

private:
    bool _value;
};

/**
 * @ingroup Either
 * Partial specialization of the Either class for void error and generic value type
 */
template <typename ValueType>
class Either<void, ValueType> : public core::Functor<Either, void, ValueType>
                              , public core::Applicative<Either, void, ValueType>
                              , public core::Monad<Either, void, ValueType> {
    friend class core::Functor<Either, void, ValueType>;
    friend class core::Applicative<Either, void, ValueType>;
    friend class core::Monad<Either, void, ValueType>;

private:
    Either() : _value{}{}
    explicit Either(const ValueType& value) : _value{value}{}

public:
    /**
     * Copy constructor
     * @param other argument to be copied
     */
    Either(const Either<void, ValueType>& other) = default;

    /**
     * Move constructor
     * @param other argument to be moved
     */
    Either(Either<void, ValueType>&& other) noexcept = default;

    /**
     * Assignment operator
     * @param other argument to be copied
     * @return Either with the value copied
     */
    Either<void, ValueType>& operator=(const Either<void, ValueType>& other) = default;

    /**
     * Move operator
     * @param other argument to be moved
     * @return Either with the value moved
     */
    Either<void, ValueType>& operator=(Either<void, ValueType>&& other) noexcept = default;

    /**
     * Comparison operator overload
     * @param other instance of either to compare to
     * @return true if objects are equal and false otherwise
     */
    bool operator==(const Either<void, ValueType>& other) const noexcept {
        return _value == other._value;
    }

    /**
     * Logical not operator
     * @return false if either has value and true otherwise
     */
    bool operator!() const {
        return !_value.has_value();
    }

    /**
     * Constructs an Either type that is an Error
     * @return Either with error defined
     */
    static Either<void, ValueType> Error() {
        return Either<void, ValueType>();
    }

    /**
     * Constructs an Either type that is a Value
     * @param value to be wrapped in the Either
     * @return Either with value defined
     */
    static Either<void, ValueType> Ok(const ValueType& value) {
        return Either<void, ValueType>(value);
    }

    /**
     * Returns whether Either is an Error or a Value
     * @return true if error and false otherwise
     */
    [[nodiscard]] bool isError() const { return !_value.has_value(); }

    /**
     * Returns whether Either is an Error or a Value
     * @return true if value and false otherwise
     */
    [[nodiscard]] bool isOk() const { return _value.has_value(); }

    /**
     * Extracts the wrapped value from the Either
     * @return the value wrapped
     * @throws std::runtime_error when either contains error
     */
    [[nodiscard]] ValueType value() const {
        if (isOk()) return _value.value();
        throw std::runtime_error("ValueType not defined");
    }

    /**
     * Extracts the wrapped value from the Either if exists or returns
     * te provided default value if either contains error
     * @param defaultValue Default value
     * @return the value wrapped or default
     */
    [[nodiscard]] ValueType valueOr(const ValueType& defaultValue) const {
        return (isOk()) ? value() : defaultValue;
    }

private:
    template <typename Callable>
    decltype(auto) internal_bind(Callable&& callable) const {
        static_assert(std::is_invocable_v<std::decay_t<Callable>, std::decay_t<ValueType>>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<Callable>, std::decay_t<ValueType>>>;
        using InnerTypeError = typename type::Details<ReturnType>::ErrorType;
        static_assert(std::is_same_v<void, InnerTypeError>, "Error type does not match");
        using InnerTypeOK = typename type::Details<ReturnType>::ValueType;
        if (isOk()) {
            return std::invoke<Callable>(std::forward<Callable>(callable), value());
        } else {
            return Either<InnerTypeError, InnerTypeOK>::Error();
        }
    }

    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<std::decay_t<Callable>, std::decay_t<ValueType>>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<Callable>, std::decay_t<ValueType>>>;
        if (isOk()) {
            if constexpr (std::is_void_v<ReturnType>) {
                std::invoke<Callable>(std::forward<Callable>(callable), value());
                return Either<void, ReturnType>::Ok();
            } else {
                return Either<void, ReturnType>::Ok(std::invoke<Callable>(std::forward<Callable>(callable), value()));
            }
        } else {
            return Either<void, ReturnType>::Error();
        }
    }

    template<typename Arg>
    decltype(auto) internal_apply(Arg&& arg) const {
        static_assert(!std::is_invocable_v<std::decay_t<ValueType>>, "Function that takes 0 arguments cannot be called with arguments");
        if constexpr (type::Details<Arg>::hasMonadicBase) {
            if (arg.isOk()) {
                return internal_apply_non_monad(arg.value());
            } else {
                using ArgInnerType = std::remove_reference_t<typename type::Details<Arg>::ValueType>;
                if constexpr (std::is_invocable_v<std::decay_t<ValueType>, std::decay_t<ArgInnerType>>) {
                    using ReturnType = std::remove_reference_t<std::invoke_result_t<ValueType, ArgInnerType>>;
                    return Either<void, ReturnType>::Error();
                } else {
                    using ReturnType = std::remove_reference_t<typename function::Details<ValueType>::PartialApplyFirst>;
                    return Either<void, ReturnType>::Error();
                }
            }
        } else {
            return internal_apply_non_monad(std::forward<Arg>(arg));
        }
    }

    template<typename Arg>
    decltype(auto) internal_apply_non_monad(Arg&& arg) const {
        static_assert(!std::is_invocable_v<std::decay_t<ValueType>>, "Function that takes 0 arguments cannot be called with arguments");

        if constexpr (std::is_invocable_v<std::decay_t<ValueType>, std::decay_t<Arg>>) {
            using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<ValueType>, std::decay_t<Arg>>>;
            if (isError()) {
                return Either<void, ReturnType>::Error();
            } else {
                if constexpr (std::is_void_v<ReturnType>) {
                    std::invoke<ValueType>(value(), std::move(arg));
                    return Either<void, ReturnType>::Ok();
                } else {
                    return Either<void, ReturnType>::Ok(std::invoke<ValueType>(value(), std::move(arg)));
                }
            }
        } else {
            using PartialFunctionType = std::remove_reference_t<typename function::Details<ValueType>::PartialApplyFirst>;

            if (isOk()) {
                return Either<void, PartialFunctionType>::Ok(
                        [callable = value(), first = std::forward<Arg>(arg)](auto&& ...args) mutable {
                            return callable(std::move(first), std::forward<decltype(args)>(args)...);
                        });
                } else {
                    return Either<void, PartialFunctionType>::Error();
                }
            }
        }

    decltype(auto) internal_apply() const {
        static_assert(std::is_invocable_v<std::decay_t<ValueType>>, "Function that takes one or more arguments and therefore cannot be called without arguments");

        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<ValueType>>>;
        if (isError()) {
            return Either<void, ReturnType>::Error();
        } else {
            if constexpr (std::is_void_v<ReturnType>) {
                std::invoke<ValueType>(value());
                return Either<void, ReturnType>::Ok();
            } else {
                return Either<void, ReturnType>::Ok(std::invoke<ValueType>(value()));
            }
        }
    }

private:
    std::optional<ValueType> _value;
};

/**
 * @ingroup Either
 * Partial specialization of the Either class for generic error and void value type
 */
template <typename ErrorType>
class Either<ErrorType, void> : public core::Functor<Either, ErrorType, void>
                              , public core::Monad<Either, ErrorType, void> {
    friend class core::Functor<Either, ErrorType, void>;
    friend class core::Monad<Either, ErrorType, void>;

private:
    Either() : _error{}{}
    explicit Either(const ErrorType& error) : _error{error}{}

public:
    /**
     * Copy constructor
     * @param other argument to be copied
     */
    Either(const Either<ErrorType, void>& other) = default;

    /**
     * Move constructor
     * @param other argument to be moved
     */
    Either(Either<ErrorType, void>&& other) noexcept = default;

    /**
     * Assignment operator
     * @param other argument to be copied
     * @return Either with the value copied
     */
    Either<ErrorType, void>& operator=(const Either<ErrorType, void>& other) = default;

    /**
     * Move operator
     * @param other argument to be moved
     * @return Either with the value moved
     */
    Either<ErrorType, void>& operator=(Either<ErrorType, void>&& other) noexcept = default;

    /**
     * Comparison operator overload
     * @param other instance of either to compare to
     * @return true if objects are equal and false otherwise
     */
    bool operator==(const Either<ErrorType, void>& other) const noexcept {
        return _error == other._error;
    }

    /**
     * Logical not operator
     * @return true if either has error and false otherwise
     */
    bool operator!() const {
        return _error.has_value();
    }

    /**
     * Constructs an Either type that is an Ok
     * @return Either with value defined
     */
    static Either<ErrorType, void> Ok() {
        return Either<ErrorType, void>();
    }

    /**
     * Constructs an Either type that is an Error
     * @param error to be wrapped as error
     * @return Either with error defined
     */
    static Either<ErrorType, void> Error(const ErrorType& error) {
        return Either<ErrorType, void>(error);
    }

    /**
     * Returns whether Either is an Error or a Value
     * @return true if error and false otherwise
     */
    [[nodiscard]] bool isError() const { return _error.has_value(); }

    /**
     * Returns whether Either is an Error or a Value
     * @return true if value and false otherwise
     */
    [[nodiscard]] bool isOk() const { return !_error.has_value(); }

    /**
     * Extracts the wrapped error from the Either
     * @return the error wrapped
     * @throws std::runtime_error when either contains value
     */
    [[nodiscard]] ErrorType error() const {
        if (isError()) return _error.value();
        throw std::runtime_error("Error not defined");
    }

    /**
     * Extracts the wrapped error from the Either if exists
     * otherwise return the provided default error
     * @param defaultError Default error
     * @return the error wrapped
     */
    [[nodiscard]] ErrorType errorOr(const ErrorType& defaultError) const {
        return isError() ? _error.value() : defaultError;
    }

private:
    template <typename Callable>
    decltype(auto) internal_bind(Callable&& callable) const {
        static_assert(std::is_invocable_v<std::decay_t<Callable>>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<Callable>>>;
        using InnerTypeError = typename type::Details<ReturnType>::ErrorType;
        static_assert(std::is_same_v<ErrorType, InnerTypeError>, "Error type does not match");
        using InnerTypeOK = typename type::Details<ReturnType>::ValueType;
        if (isOk()) {
            return std::invoke<Callable>(std::forward<Callable>(callable));
        } else {
            return Either<ErrorType, InnerTypeOK>::Error(error());
        }
    }

    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<std::decay_t<Callable>>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<Callable>>>;
        if (isOk()) {
            if constexpr (std::is_void_v<ReturnType>) {
                std::invoke<Callable>(std::forward<Callable>(callable));
                return Either<ErrorType, ReturnType>::Ok();
            } else {
                return Either<ErrorType, ReturnType>::Ok(std::invoke<Callable>(std::forward<Callable>(callable)));
            }
        } else {
            return Either<ErrorType, ReturnType>::Error(error());
        }
    }

private:
    std::optional<ErrorType> _error;
};

/**
 * @ingroup Either
 * Generalization of the Either class for any error and value types other than void
 */
template <typename ErrorType, typename ValueType>
class Either : public core::Functor<Either, ErrorType, ValueType>
             , public core::Applicative<Either, ErrorType, ValueType>
             , public core::Monad<Either, ErrorType, ValueType> {
    friend class core::Functor<Either, ErrorType, ValueType>;
    friend class core::Applicative<Either, ErrorType, ValueType>;
    friend class core::Monad<Either, ErrorType, ValueType>;

private:
    enum Type {
        EitherError = 0,
        EitherValue = 1
    };

    explicit Either(Type type): _type{type}{}

public:
    /**
     * Copy constructor
     * @param other argument to be copied
     */
    Either(const Either<ErrorType, ValueType>& other) = default;

    /**
     * Move constructor
     * @param other argument to be moved
     */
    Either(Either<ErrorType, ValueType>&& other) noexcept = default;

    /**
     * Assignment operator
     * @param other argument to be copied
     * @return Either with the value copied
     */
    Either<ErrorType, ValueType>& operator=(const Either<ErrorType, ValueType>& other) = default;

    /**
     * Move operator
     * @param other argument to be moved
     * @return Either with the value moved
     */
    Either<ErrorType, ValueType>& operator=(Either<ErrorType, ValueType>&& other) noexcept = default;

    /**
     * Comparison operator overload
     * @param other instance of either to compare to
     * @return true if objects are equal and false otherwise
     */
    bool operator==(const Either<ErrorType, ValueType>& other) const noexcept {
        return _type == other._type && _value == other._value;
    }

    /**
     * Logical not operator
     * @return false if either has value and true otherwise
     */
    bool operator!() const {
        return (_type == Type::EitherValue) ? false : true;
    }

    /**
     * Constructs an Either type that is an Error
     * @param value to be wrapped as error
     * @return Either with error defined
     */
    static Either<ErrorType, ValueType> Error(const ErrorType& value) {
        Either<ErrorType, ValueType> result{Type::EitherError};
        result._value.template emplace<Type::EitherError>(value);
        return result;
    }

    /**
     * Constructs an Either type that is a Value
     * @param value to be wrapped in the Either
     * @return Either with value defined
     */
    static Either<ErrorType,ValueType> Ok(const ValueType& value) {
        Either<ErrorType, ValueType> result{Type::EitherValue};
        result._value.template emplace<Type::EitherValue>(value);
        return result;
    }

    /**
     * Returns whether Either is an Error or a Value
     * @return true if error and false otherwise
     */
    [[nodiscard]] bool isError() const { return _type == Type::EitherError; }

    /**
     * Returns whether Either is an Error or a Value
     * @return true if value and false otherwise
     */
    [[nodiscard]] bool isOk() const { return _type == Type::EitherValue; }

    /**
     * Extracts the wrapped error from the Either
     * @return the error wrapped
     * @throws std::runtime_error when either contains value
     */
    [[nodiscard]] ErrorType error() const {
        if (isError()) return std::get<Type::EitherError>(_value);
        throw std::runtime_error("Error not defined");
    }

    /**
     * Extracts the wrapped value from the Either
     * @return the value wrapped
     * @throws std::runtime_error when either contains error
     */
    [[nodiscard]] ValueType value() const {
        if (isOk()) return std::get<Type::EitherValue>(_value);
        throw std::runtime_error("Ok not defined");
    }

    /**
     * Extracts the wrapped value from the Either if exists or returns
     * te provided default value if either contains error
     * @param defaultValue Default value
     * @return the wrapped value or default
     */
    [[nodiscard]] ValueType valueOr(const ValueType& defaultValue) const {
        return (isOk()) ? std::get<Type::EitherValue>(_value) : defaultValue;
    }

    /**
     * Extracts the wrapped error from the Either if exists or returns
     * te provided default error if either contains a value
     * @param defaultError Default value
     * @return the wrapped error or default
     */
    [[nodiscard]] ErrorType errorOr(const ErrorType& defaultError) const {
        return (isError()) ? std::get<Type::EitherError>(_value) : defaultError;
    }

private:
    template <typename Callable>
    decltype(auto) internal_bind(Callable&& callable) const {
        static_assert(std::is_invocable_v<std::decay_t<Callable>, std::decay_t<ValueType>>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<Callable>, std::decay_t<ValueType>>>;
        using InnerTypeError = typename type::Details<ReturnType>::ErrorType;
        static_assert(std::is_same_v<ErrorType, InnerTypeError>, "Error type does not match");
        using InnerTypeOK = typename type::Details<ReturnType>::ValueType;
        if (isOk()) {
            return std::invoke<Callable>(std::forward<Callable>(callable), value());
        } else {
            return Either<ErrorType, InnerTypeOK>::Error(error());
        }
    }

    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<std::decay_t<Callable>, std::decay_t<ValueType>>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<Callable>, std::decay_t<ValueType>>>;
        if (isOk()) {
            if constexpr (std::is_void_v<ReturnType>) {
                std::invoke<Callable>(std::forward<Callable>(callable), value());
                return Either<ErrorType, ReturnType>::Ok();
            } else {
                return Either<ErrorType, ReturnType>::Ok(std::invoke<Callable>(std::forward<Callable>(callable), value()));
            }
        } else {
            return Either<ErrorType, ReturnType>::Error(error());
        }
    }

    template<typename Arg>
    decltype(auto) internal_apply(Arg&& arg) const {
        static_assert(!std::is_invocable_v<std::decay_t<ValueType>>, "Function that takes 0 arguments cannot be called with arguments");
        if constexpr (type::Details<Arg>::hasMonadicBase) {
            if (arg.isOk()) {
                return internal_apply_non_monad(arg.value());
            } else {
                using ArgInnerType = std::remove_reference_t<typename type::Details<Arg>::ValueType>;
                if constexpr (std::is_invocable_v<std::decay_t<ValueType>, std::decay_t<ArgInnerType>>) {
                    using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<ValueType>, std::decay_t<ArgInnerType>>>;
                    return Either<ErrorType, ReturnType>::Error(arg.error());
                } else {
                    using ReturnType = std::remove_reference_t<typename function::Details<ValueType>::PartialApplyFirst>;
                    return Either<ErrorType, ReturnType>::Error(arg.error());
                }
            }
        } else {
            return internal_apply_non_monad(std::forward<Arg>(arg));
        }
    }

    template<typename Arg>
    decltype(auto) internal_apply_non_monad(Arg&& arg) const {
        if constexpr (std::is_invocable_v<std::decay_t<ValueType>, std::decay_t<Arg>>) {
            using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<ValueType>, std::decay_t<Arg>>>;
            if (isError()) {
                return Either<ErrorType, ReturnType>::Error(error());
            } else {
                if constexpr (std::is_void_v<ReturnType>) {
                    std::invoke<ValueType>(value(), std::move(arg));
                    return Either<ErrorType, ReturnType>::Ok();
                } else {
                    return Either<ErrorType, ReturnType>::Ok(std::invoke<ValueType>(value(), std::move(arg)));
                }
            }
        } else {
            using PartialFunctionType = std::remove_reference_t<typename function::Details<ValueType>::PartialApplyFirst>;

            if (isOk()) {
                return Either<ErrorType, PartialFunctionType>::Ok([callable = value(), first = std::forward<Arg>(arg)](auto&& ...args) {
                    return std::apply(callable, std::tuple_cat(std::make_tuple(first), std::make_tuple(args...)));
                        });
            } else {
                return Either<ErrorType, PartialFunctionType>::Error(this->error());
            }
        }
    }

    decltype(auto) internal_apply() const {
        static_assert(std::is_invocable_v<std::decay_t<ValueType>>, "Function that takes one or more arguments and therefore cannot be called without arguments");
        if constexpr (std::is_invocable_v<std::decay_t<ValueType>>) {
            using ReturnType = std::remove_reference_t<std::invoke_result_t<std::decay_t<ValueType>>>;
            if (isError()) {
                return Either<ErrorType, ReturnType>::Error(error());
            } else {
                if constexpr (std::is_void_v<ReturnType>) {
                    std::invoke<ValueType>(value());
                    return Either<ErrorType, ReturnType>::Ok();
                } else {
                    return Either<ErrorType, ReturnType>::Ok(std::invoke<ValueType>(value()));
                }
            }
        } else {
            return Either<ErrorType, ValueType>::Error(error());
        }
    }

private:
    std::variant<ErrorType, ValueType> _value;
    Type _type;
};

namespace either {

/**
 * @ingroup Either
 * Helper function to create an Either that is an Ok.
 * Is this case this function is applicable when both error and value types are void
 * @tparam ErrorType Type of error
 * @tparam ValueType Type of value
 * @return valid value Either
 */
template<typename ErrorType, typename ValueType>
std::enable_if_t<std::is_void_v<ValueType> && std::is_void_v<ErrorType>, Either<void, void>>
Ok() { return Either<void, void>::Ok(); }

/**
 * @ingroup Either
 * Helper function to create an Either that is an Ok.
 * Is this case this function is applicable when error type is not void and value type is void
 * @tparam ErrorType Type of error
 * @tparam ValueType Type of value
 * @return valid value Either
 */
template<typename ErrorType, typename ValueType>
std::enable_if_t<(!std::is_void_v<ErrorType>) && std::is_void_v<ValueType>, Either<ErrorType, void>>
Ok() { return Either<ErrorType, void>::Ok(); }

/**
 * @ingroup Either
 * Helper function to create an Either that is an Ok.
 * Is this case this function is applicable when error type is not void and value type is not void
 * @tparam ErrorType Type of error
 * @tparam ValueType Type of value
 * @param arg Value to be wrapped
 * @return valid value Either
 */
template<typename ErrorType, typename ValueType>
std::enable_if_t<!std::is_void_v<ErrorType> && !std::is_void_v<ValueType>, Either<ErrorType, ValueType>>
Ok(const ValueType& arg) { return Either<ErrorType, ValueType>::Ok(arg); }

/**
 * @ingroup Either
 * Helper function to create an Either that is an Ok.
 * Is this case this function is applicable when error type is void and value type is not void
 * @tparam ErrorType Type of error
 * @tparam ValueType Type of value
 * @param arg Value to be wrapped
 * @return valid value Either
 */
template<typename ErrorType, typename ValueType>
std::enable_if_t<std::is_void_v<ErrorType> && !std::is_void_v<ValueType>, Either<void, ValueType>>
Ok(const ValueType& arg) { return Either<void, ValueType>::Ok(arg); }

/**
 * @ingroup Either
 * Helper function to create an Either that is an Error.
 * Is this case this function is applicable when both error and value types are void
 * @tparam ErrorType Type of error
 * @tparam ValueType Type of value
 * @return valid eror Either
 */
template<typename ErrorType, typename ValueType>
std::enable_if_t<std::is_void_v<ErrorType> && std::is_void_v<ValueType>, Either<void, void>>
Error() { return Either<void, void>::Error(); }

/**
 * @ingroup Either
 * Helper function to create an Either that is an Error.
 * Is this case this function is applicable when error type is not void and value type is void
 * @tparam ErrorType Type of error
 * @tparam ValueType Type of value
 * @param arg Error to be wrapped
 * @return valid error Either
 */
template<typename ErrorType, typename ValueType>
std::enable_if_t<!std::is_void_v<ErrorType> && std::is_void_v<ValueType>, Either<ErrorType, void>>
Error(const ErrorType& arg) { return Either<ErrorType, void>::Error(arg); }

/**
 * @ingroup Either
 * Helper function to create an Either that is an Error.
 * Is this case this function is applicable when error type is not void and value type is not void
 * @tparam ErrorType Type of error
 * @tparam ValueType Type of value
 * @param arg Error to be wrapped
 * @return valid error Either
 */
template<typename ErrorType, typename ValueType>
std::enable_if_t<!std::is_void_v<ErrorType> && !std::is_void_v<ValueType>, Either<ErrorType, ValueType>>
Error(const ErrorType& arg) { return Either<ErrorType, ValueType>::Error(arg); }

/**
 * @ingroup Either
 * Helper function to create an Either that is an Error.
 * Is this case this function is applicable when error type is void and value type is not void
 * @tparam ErrorType Type of error
 * @tparam ValueType Type of value
 * @return valid error Either
 */
template<typename ErrorType, typename ValueType>
std::enable_if_t<std::is_void_v<ErrorType> && !std::is_void_v<ValueType>, Either<void, ValueType>>
Error() { return Either<void, ValueType>::Error(); }

/**
 * @ingroup Either
 * Lift given callable into the given abstract monadic type
 * @tparam MonadType Abstract type to lift function to
 * @tparam Callable Callable type to lift
 * @param callable Callable to lift
 * @return callable lifted into the given abstract monadic type
 */
template<typename Callable>
decltype(auto) lift(Callable&& callable) {
    if constexpr (std::is_invocable_v<Callable>) {
        using ReturnType = std::remove_reference_t<std::invoke_result_t<Callable>>;
        if constexpr (std::is_void_v<ReturnType>) {
            return [callable = std::forward<Callable>(callable)](){
                callable();
                return Either<void, ReturnType>::Ok();
            };
        } else {
            return [callable = std::forward<Callable>(callable)](){
                return Either<void, ReturnType>::Ok(callable());
            };
        }
    } else {
        using ReturnType = typename function::Details<Callable>::ReturnType;
        using FixedErrorType = typename type::FixedErrorType<void>;
        using ReturnFunctionType = typename function::Details<Callable>::template LiftedSignature<FixedErrorType::template Type>;

        const ReturnFunctionType function =  [callable = std::forward<Callable>(callable)](auto ...args) -> Either<void, ReturnType> {
            if (all_true([](auto&& v){ return v.isOk();}, args...)) {
                const auto tp = tuple::map_append([](auto&& arg){ return arg.value();}, std::make_tuple(), args...);

                if constexpr (std::is_void_v<ReturnType>) {
                    std::apply(callable, tp);
                    return Either<void, ReturnType>::Ok();
                } else {
                    return Either<void, ReturnType>::Ok(std::apply(callable, tp));
                }
            } else {
                return Either<void, ReturnType>::Error();
            }
        };

        return function;
    }
}

} // namespace either
} // namespace yafl
