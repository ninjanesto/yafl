/**
 * \brief       Yet Another Functional Library
 *
 * \copyright   Critical TechWorks SA
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
    
namespace either {
/**
 * @ingroup Either
 * Either monad traits
 */
template<typename>
struct Details;

/**
 * @ingroup Either
 * Either monad traits specialization that enable getting the inner error and value types
 * @tparam EitherT Either monad type
 * @tparam InnerError Error type
 * @tparam InnerValue Value type
 */
template<template<typename, typename> typename EitherT, typename InnerError, typename InnerValue>
struct Details<EitherT<InnerError, InnerValue>> {
    using ErrorType = InnerError;
    using ValueType = InnerValue;
};

/**
 * @ingroup Either
 * Either monad traits specialization for const types that enable getting the inner error and value types
 * @tparam EitherT Either monad type
 * @tparam InnerError Error type
 * @tparam InnerValue Value type
 */
template<template<typename, typename> typename EitherT, typename InnerError, typename InnerValue>
struct Details<const EitherT<InnerError, InnerValue>> {
    using ErrorType = InnerError;
    using ValueType = InnerValue;
};

} // namespace either

/**
 * @ingroup Either
 * Either class. This class implements the Either type by realizing concepts from functional programming
 * such as Functor, Applicative and Monad. The implementation for Either class was based on Haskell Either
 * type as defined in https://hackage.haskell.org/package/base-4.18.0.0/docs/Data-Either.html
 */
template<typename Error, typename Value>
class Either;

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
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<Callable>>;
        if (this->isOk()) {
            if constexpr (std::is_void_v<ReturnType>) {
                callable();
                return Either<void, ReturnType>::Ok();
            } else {
                return Either<void, ReturnType>::Ok(callable());
            }
        } else {
            return Either<void, ReturnType>::Error();
        }
    }

    template <typename Callable>
    decltype(auto) internal_bind(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<Callable>>;
        using InnerTypeError = typename either::Details<ReturnType>::ErrorType;
        static_assert(std::is_same_v<void, InnerTypeError>, "Error type does not match");
        using InnerTypeOK = typename either::Details<ReturnType>::ValueType;
        if (this->isOk()) {
            return callable();
        } else {
            return Either<InnerTypeError, InnerTypeOK>::Error();
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
    explicit Either(bool isError) : _isError{isError} {}

public:
    /**
     * Constructs an Either type that is an Error
     * @return Either with error defined
     */
    static Either<void, ValueType> Error() {
        return Either<void, ValueType>(true);
    }

    /**
     * Constructs an Either type that is a Value
     * @param value to be wrapped in the Either
     * @return Either with value defined
     */
    static Either<void, ValueType> Ok(const ValueType& value) {
        Either<void, ValueType> result(false);
        result._right = std::make_unique<ValueType>(value);
        return result;
    }

    /**
     * Returns whether Either is an Error or a Value
     * @return true if error and false otherwise
     */
    [[nodiscard]] bool isError() const { return _isError; }

    /**
     * Returns whether Either is an Error or a Value
     * @return true if value and false otherwise
     */
    [[nodiscard]] bool isOk() const { return !_isError; }

    /**
     * Extracts the wrapped value from the Either
     * @return the value wrapped
     * @throws std::runtime_error when either contains error
     */
    ValueType value() const {
        if (!_isError) return *(_right.get());
        throw std::runtime_error("Ok not defined");
    }

private:
    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable, ValueType>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<Callable, ValueType>>;
        if (this->isOk()) {
            if constexpr (std::is_void_v<ReturnType>) {
                callable(value());
                return Either<void, ReturnType>::Ok();
            } else {
                return Either<void, ReturnType>::Ok(callable(value()));
            }
        } else {
            return Either<void, ReturnType>::Error();
        }
    }

    template <typename Callable>
    decltype(auto) internal_bind(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable, ValueType>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<Callable, ValueType>>;
        using InnerTypeError = typename either::Details<ReturnType>::ErrorType;
        static_assert(std::is_same_v<void, InnerTypeError>, "Error type does not match");
        using InnerTypeOK = typename either::Details<ReturnType>::ValueType;
        if (this->isOk()) {
            return callable(this->value());
        } else {
            return Either<InnerTypeError, InnerTypeOK>::Error();
        }
    }

    template<typename Head>
    decltype(auto) internal_apply(Either<void, Head>&& arg) const {
        const auto var{std::move(arg)};
        if constexpr (std::is_invocable_v<ValueType, Head>) {
            using ReturnType = std::remove_reference_t<std::invoke_result_t<ValueType, Head>>;
            if (this->isError()) {
                return Either<void, ReturnType>::Error();
            } else {
                if (var.isOk()) {
                    if constexpr (std::is_void_v<ReturnType>) {
                        value()(var.value());
                        return Either<void, ReturnType>::Ok();
                    } else {
                        return Either<void, ReturnType>::Ok(value()(var.value()));
                    }
                } else {
                    return Either<void, ReturnType>::Error();
                }
            }
        } else {
            if (var.isOk() && this->isOk()) {
                return Either<void, typename FunctionTraits<ValueType>::PartialApplyFirst>::Ok([callable = value(), first = var.value()](auto&& ...args) {
                    return std::apply(callable, std::tuple_cat(std::make_tuple(first), std::make_tuple(args...)));
                });
            } else {
                return Either<void, typename FunctionTraits<ValueType>::PartialApplyFirst>::Error();
            }
        }
    }

    template<typename Head>
    decltype(auto) internal_apply(Head&& arg) const {
        if constexpr (std::is_invocable_v<ValueType, Head>) {
            using ReturnType = std::remove_reference_t<std::invoke_result_t<ValueType, Head>>;
            if (this->isError()) {
                return Either<void, ReturnType>::Error();
            } else {
                if constexpr (std::is_void_v<ReturnType>) {
                    value()(std::forward<Head>(arg));
                    return Either<void, ReturnType>::Ok();
                } else {
                    return Either<void, ReturnType>::Ok(value()(std::forward<Head>(arg)));
                }
            }
        } else {
            if (this->isOk()) {
                return Either<void, typename FunctionTraits<ValueType>::PartialApplyFirst>::Ok([callable = value(), first = std::forward<Head>(arg)](auto&& ...args) {
                    return std::apply(callable, std::tuple_cat(std::make_tuple(first), std::make_tuple(args...)));
                });
            } else {
                return Either<void, typename FunctionTraits<ValueType>::PartialApplyFirst>::Error();
            }
        }
    }

    template<typename std::enable_if<std::is_invocable_v<ValueType>>* = nullptr>
    decltype(auto) internal_apply() const {
        using ReturnType = std::remove_reference_t<std::invoke_result_t<ValueType>>;
        if (this->isError()) {
            return Either<void, ReturnType>::Error();
        } else {
            if constexpr (std::is_void_v<ReturnType>) {
                value()();
                return Either<void, ReturnType>::Ok();
            } else {
                return Either<void, ReturnType>::Ok(value()());
            }
        }
    }

private:
    std::unique_ptr<ValueType> _right{nullptr};
    bool _isError;
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
    explicit Either(bool isError) : _isError{isError} {}

public:
    /**
     * Constructs an Either type that is an Ok
     * @return Either with value defined
     */
    static Either<ErrorType, void> Ok() {
        return Either<ErrorType, void>(false);
    }

    /**
     * Constructs an Either type that is an Error
     * @param value to be wrapped as error
     * @return Either with error defined
     */
    static Either<ErrorType, void> Error(const ErrorType& value) {
        Either<ErrorType, void> result(true);
        result._left = std::make_unique<ErrorType>(value);
        return result;
    }

    /**
     * Returns whether Either is an Error or a Value
     * @return true if error and false otherwise
     */
    [[nodiscard]] bool isError() const { return _isError; }

    /**
     * Returns whether Either is an Error or a Value
     * @return true if value and false otherwise
     */
    [[nodiscard]] bool isOk() const { return !_isError; }

    /**
     * Extracts the wrapped error from the Either
     * @return the error wrapped
     * @throws std::runtime_error when either contains value
     */
    ErrorType error() const {
        if (_isError) return *(_left.get());
        throw std::runtime_error("Ok not defined");
    }

private:
    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<Callable>>;
        if (this->isOk()) {
            if constexpr (std::is_void_v<ReturnType>) {
                callable();
                return Either<ErrorType, ReturnType>::Ok();
            } else {
                return Either<ErrorType, ReturnType>::Ok(callable());
            }
        } else {
            return Either<ErrorType, ReturnType>::Error(error());
        }
    }

    template <typename Callable>
    decltype(auto) internal_bind(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<Callable>>;
        using InnerTypeError = typename either::Details<ReturnType>::ErrorType;
        static_assert(std::is_same_v<ErrorType, InnerTypeError>, "Error type does not match");
        using InnerTypeOK = typename either::Details<ReturnType>::ValueType;
        if (this->isOk()) {
            return callable();
        } else {
            return Either<InnerTypeError, InnerTypeOK>::Error(this->error());
        }
    }

private:
    std::unique_ptr<ErrorType> _left{nullptr};
    bool _isError;
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
    explicit Either(bool isError): _isError{isError}{}

public:
    /**
     * Constructs an Either type that is an Error
     * @param value to be wrapped as error
     * @return Either with error defined
     */
    static Either<ErrorType, ValueType> Error(const ErrorType& value) {
        Either<ErrorType, ValueType> result{true};
        result._value.template emplace<0>(value);
        return result;
    }

    /**
     * Constructs an Either type that is a Value
     * @param value to be wrapped in the Either
     * @return Either with value defined
     */
    static Either<ErrorType,ValueType> Ok(const ValueType& value) {
        Either<ErrorType, ValueType> result{false};
        result._value.template emplace<1>(value);
        return result;
    }

    /**
     * Returns whether Either is an Error or a Value
     * @return true if error and false otherwise
     */
    [[nodiscard]] bool isError() const { return _isError; }

    /**
     * Returns whether Either is an Error or a Value
     * @return true if value and false otherwise
     */
    [[nodiscard]] bool isOk() const { return !_isError; }

    /**
     * Extracts the wrapped error from the Either
     * @return the error wrapped
     * @throws std::runtime_error when either contains value
     */
    ErrorType error() const {
        if (isError()) return std::get<0>(_value);
        throw std::runtime_error("Error not defined");
    }

    /**
     * Extracts the wrapped value from the Either
     * @return the value wrapped
     * @throws std::runtime_error when either contains error
     */
    ValueType value() const {
        if (isOk()) return std::get<1>(_value);
        throw std::runtime_error("Ok not defined");
    }

private:
    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable, ValueType>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<Callable, ValueType>>;
        if (this->isOk()) {
            if constexpr (std::is_void_v<ReturnType>) {
                callable(value());
                return Either<ErrorType, ReturnType>::Ok();
            } else {
                return Either<ErrorType, ReturnType>::Ok(callable(value()));
            }
        } else {
            return Either<ErrorType, ReturnType>::Error(error());
        }
    }

    template <typename Callable>
    decltype(auto) internal_bind(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable, ValueType>, "Input argument is not invocable");
        using ReturnType = std::remove_reference_t<std::invoke_result_t<Callable, ValueType>>;
        using InnerTypeError = typename either::Details<ReturnType>::ErrorType;
        static_assert(std::is_same_v<ErrorType, InnerTypeError>, "Error type does not match");
        using InnerTypeOK = typename either::Details<ReturnType>::ValueType;
        if (this->isOk()) {
            return callable(this->value());
        } else {
            return Either<ErrorType, InnerTypeOK>::Error(this->error());
        }
    }

    template<typename Head>
    decltype(auto) internal_apply(Either<ErrorType, Head>&& arg) const {
        const auto var{std::move(arg)};
        if constexpr (std::is_invocable_v<ValueType, Head>) {
            using ReturnType = std::remove_reference_t<std::invoke_result_t<ValueType, Head>>;
            if (this->isError()) {
                return Either<ErrorType, ReturnType>::Error(this->error());
            } else {
                if (var.isOk()) {
                    if constexpr (std::is_void_v<ReturnType>) {
                        value()(var.value());
                        return Either<ErrorType, ReturnType>::Ok();
                    } else {
                        return Either<ErrorType, ReturnType>::Ok(value()(var.value()));
                    }
                } else {
                    return Either<ErrorType, ReturnType>::Error(var.error());
                }
            }
        } else {
            if (this->isOk()) {
                if (var.isOk()) {
                    return Either<ErrorType, typename FunctionTraits<ValueType>::PartialApplyFirst>::Ok([callable = value(), first = var.value()](auto&& ...args) {
                        return std::apply(callable, std::tuple_cat(std::make_tuple(first), std::make_tuple(args...)));
                    });
                } else {
                    return Either<ErrorType, typename FunctionTraits<ValueType>::PartialApplyFirst>::Error(var.error());
                }
            } else {
                return Either<ErrorType, typename FunctionTraits<ValueType>::PartialApplyFirst>::Error(this->error());
            }
        }
    }

    template<typename Head>
    decltype(auto) internal_apply(Head&& arg) const {
        if constexpr (std::is_invocable_v<ValueType, Head>) {
            using ReturnType = std::remove_reference_t<std::invoke_result_t<ValueType, Head>>;
            if (this->isError()) {
                return Either<ErrorType, ReturnType>::Error(this->error());
            } else {
                if constexpr (std::is_void_v<ReturnType>) {
                    value()(std::forward<Head>(arg));
                    return Either<ErrorType, ReturnType>::Ok();
                } else {
                    return Either<ErrorType, ReturnType>::Ok(value()(std::forward<Head>(arg)));
                }
            }
        } else {
            if (this->isOk()) {
                return Either<ErrorType, typename FunctionTraits<ValueType>::PartialApplyFirst>::Ok([callable = value(), first = std::forward<Head>(arg)](auto&& ...args) {
                    return std::apply(callable, std::tuple_cat(std::make_tuple(first), std::make_tuple(args...)));
                });
            } else {
                return Either<ErrorType, typename FunctionTraits<ValueType>::PartialApplyFirst>::Error(this->error());
            }
        }
    }

    template<typename std::enable_if<std::is_invocable_v<ValueType>>* = nullptr>
    decltype(auto) internal_apply() const {
        using ReturnType = std::remove_reference_t<std::invoke_result_t<ValueType>>;
        if (this->isError()) {
            return Either<ErrorType, ReturnType>::Error(this->error());
        } else {
            if constexpr (std::is_void_v<ReturnType>) {
                value()();
                return Either<ErrorType, ReturnType>::Ok();
            } else {
                return Either<ErrorType, ReturnType>::Ok(value()());
            }
        }
    }

private:
    std::variant<ErrorType, ValueType> _value;
    bool _isError;
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
        using ReturnType = typename yafl::FunctionTraits<Callable>::ReturnType;

        return [callable = std::forward<Callable>(callable)](auto ...args) -> Either<void, ReturnType> {
            if (detail::all_true([](auto&& v){ return v.isOk();}, args...)) {
                const auto tp = detail::map_tuple_append([](auto&& arg){ return arg.value();}, std::make_tuple(), args...);

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
    }
}

} // namespace either
} // namespace yafl
