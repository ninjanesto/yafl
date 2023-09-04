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
#include <variant>
#include "yafl/Functor.h"
#include "yafl/Monad.h"
#include "yafl/Applicative.h"

namespace yafl {

template <typename T>
struct EitherTraits;

template <template <typename, typename> typename Either, typename InnerError, typename InnerValue>
struct EitherTraits<Either<InnerError, InnerValue>> {
    using ErrorType = InnerError;
    using ValueType = InnerValue;
};
template <template <typename, typename> typename Either, typename InnerError, typename InnerValue>
struct EitherTraits<const Either<InnerError, InnerValue>> {
    using ErrorType = InnerError;
    using ValueType = InnerValue;
};

// Either Monad
template<typename Error, typename Value>
class Either;


template<>
class Either<void, void> : public Functor<Either, void, void>
                         , public Monad<Either, void, void> {
    friend class Functor<Either, void, void>;
    friend class Monad<Either, void, void>;
private:
    explicit Either(bool v) : _value(v) {}
public:
    static Either<void, void> Error() {
        return Either<void, void>(false);
    }

    static Either<void, void> Ok() {
        return Either<void, void>{true};
    }

    [[nodiscard]] bool isError() const { return !_value; }

    [[nodiscard]] bool isOk() const { return _value; }

private:
    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable>, "Input argument is not invocable");
        using ReturnType = std::invoke_result_t<Callable>;
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
        using ReturnType = std::invoke_result_t<Callable>;
        using InnerTypeError = typename EitherTraits<ReturnType>::ErrorType;
        static_assert(std::is_same_v<void, InnerTypeError>, "Error type does not match");
        using InnerTypeOK = typename EitherTraits<ReturnType>::ValueType;
        if (this->isOk()) {
            return callable();
        } else {
            return Either<InnerTypeError, InnerTypeOK>::Error();
        }
    }
private:
    bool _value;
};

template <typename ValueType>
class Either<void, ValueType> : public Functor<Either, void, ValueType>
                              , public Applicative<Either, void, ValueType>
                              , public Monad<Either, void, ValueType> {
    friend class Functor<Either, void, ValueType>;
    friend class Applicative<Either, void, ValueType>;
    friend class Monad<Either, void, ValueType>;

private:
    explicit Either(bool isError) : _isError{isError} {}
public:
    static Either<void, ValueType> Error() {
        return Either<void, ValueType>(true);
    }

    static Either<void, ValueType> Ok(const ValueType& value) {
        Either<void, ValueType> result(false);
        result._right = std::make_unique<ValueType>(value);
        return result;
    }

    [[nodiscard]] bool isError() const { return _isError; }

    [[nodiscard]] bool isOk() const { return !_isError; }

    ValueType value() const {
        if (!_isError) return *(_right.get());
        throw std::runtime_error("Ok not defined");
    }
private:
    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable, ValueType>, "Input argument is not invocable");
        using ReturnType = std::invoke_result_t<Callable, ValueType>;
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
        using ReturnType = std::invoke_result_t<Callable, ValueType>;
        using InnerTypeError = typename EitherTraits<ReturnType>::ErrorType;
        static_assert(std::is_same_v<void, InnerTypeError>, "Error type does not match");
        using InnerTypeOK = typename EitherTraits<ReturnType>::ValueType;
        if (this->isOk()) {
            return callable(this->value());
        } else {
            return Either<InnerTypeError, InnerTypeOK>::Error();
        }
    }

    template<typename Head>
    decltype(auto) internal_apply(const Either<void, Head>& arg) const {
        if constexpr (function_traits<ValueType>::ArgCount > 1) {
            if (arg.isOk() && this->isOk()) {
                return Either<void, typename function_traits<ValueType>::PartialApplyFirst>::Ok([callable = value(), first = arg.value()](const auto& ...args) {
                    return callable(first, args...);
                });
            } else {
                return Either<void, typename function_traits<ValueType>::PartialApplyFirst>::Error();
            }
        } else {
            using ReturnType = std::invoke_result_t<ValueType, Head>;
            if (this->isError()) {
                return Either<void, ReturnType>::Error();
            } else {
                if (arg.isOk()) {
                    if constexpr (std::is_void_v<ReturnType>) {
                        value()(arg.value());
                        return Either<void, ReturnType>::Ok();
                    } else {
                        return Either<void, ReturnType>::Ok(value()(arg.value()));
                    }
                } else {
                    return Either<void, ReturnType>::Error();
                }
            }
        }
    }

    template<typename std::enable_if<std::is_invocable_v<ValueType>>* = nullptr>
    decltype(auto) internal_apply() const {
        using ReturnType = std::invoke_result_t<ValueType>;
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


template <typename ErrorType>
class Either<ErrorType, void> : public Functor<Either, ErrorType, void>
                              , public Monad<Either, ErrorType, void> {
    friend class Functor<Either, ErrorType, void>;
    friend class Monad<Either, ErrorType, void>;
private:
    explicit Either(bool isError) : _isError{isError} {}
public:
    static Either<ErrorType, void> Ok() {
        return Either<ErrorType, void>(false);
    }

    static Either<ErrorType, void> Error(const ErrorType& value) {
        Either<ErrorType, void> result(true);
        result._left = std::make_unique<ErrorType>(value);
        return result;
    }

    [[nodiscard]] bool isError() const { return _isError; }

    [[nodiscard]] bool isOk() const { return !_isError; }

    ErrorType error() const {
        if (_isError) return *(_left.get());
        throw std::runtime_error("Ok not defined");
    }
private:
    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable>, "Input argument is not invocable");
        using ReturnType = std::invoke_result_t<Callable>;
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
        using ReturnType = std::invoke_result_t<Callable>;
        using InnerTypeError = typename EitherTraits<ReturnType>::ErrorType;
        static_assert(std::is_same_v<ErrorType, InnerTypeError>, "Error type does not match");
        using InnerTypeOK = typename EitherTraits<ReturnType>::ValueType;
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

template <typename ErrorType, typename ValueType>
class Either : public Functor<Either, ErrorType, ValueType>
             , public Applicative<Either, ErrorType, ValueType>
             , public Monad<Either, ErrorType, ValueType> {
    friend class Functor<Either, ErrorType, ValueType>;
    friend class Applicative<Either, ErrorType, ValueType>;
    friend class Monad<Either, ErrorType, ValueType>;
private:
    explicit Either(bool isError): _isError{isError}{}
public:
    static Either<ErrorType, ValueType> Error(const ErrorType& value) {
        Either<ErrorType, ValueType> result{true};
        result._value.template emplace<0>(value);
        return result;
    }

    static Either<ErrorType,ValueType> Ok(const ValueType& value) {
        Either<ErrorType, ValueType> result{false};
        result._value.template emplace<1>(value);
        return result;
    }

    [[nodiscard]] bool isError() const { return _isError; }

    [[nodiscard]] bool isOk() const { return !_isError; }

    ErrorType error() const {
        if (isError()) return std::get<0>(_value);
        throw std::runtime_error("Error not defined");
    }

    ValueType value() const {
        if (isOk()) return std::get<1>(_value);
        throw std::runtime_error("Ok not defined");
    }
private:
    template <typename Callable>
    decltype(auto) internal_fmap(Callable&& callable) const {
        static_assert(std::is_invocable_v<Callable, ValueType>, "Input argument is not invocable");
        using ReturnType = std::invoke_result_t<Callable, ValueType>;
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
        using ReturnType = std::invoke_result_t<Callable, ValueType>;
        using InnerTypeError = typename EitherTraits<ReturnType>::ErrorType;
        static_assert(std::is_same_v<ErrorType, InnerTypeError>, "Error type does not match");
        using InnerTypeOK = typename EitherTraits<ReturnType>::ValueType;
        if (this->isOk()) {
            return callable(this->value());
        } else {
            return Either<ErrorType, InnerTypeOK>::Error(this->error());
        }
    }

    template<typename Head>
    decltype(auto) internal_apply(const Either<ErrorType, Head>& arg) const {
        if constexpr (function_traits<ValueType>::ArgCount > 1) {
            if (this->isOk()) {
                if (arg.isOk()) {
                    return Either<ErrorType, typename function_traits<ValueType>::PartialApplyFirst>::Ok([callable = value(), first = arg.value()](const auto& ...args) {
                        return callable(first, args...);
                    });
                } else {
                    return Either<ErrorType, typename function_traits<ValueType>::PartialApplyFirst>::Error(arg.error());
                }
            } else {
                return Either<ErrorType, typename function_traits<ValueType>::PartialApplyFirst>::Error(this->error());
            }
        } else {
            using ReturnType = std::invoke_result_t<ValueType, Head>;
            if (this->isError()) {
                return Either<ErrorType, ReturnType>::Error(this->error());
            } else {
                if (arg.isOk()) {
                    if constexpr (std::is_void_v<ReturnType>) {
                        value()(arg.value());
                        return Either<ErrorType, ReturnType>::Ok();
                    } else {
                        return Either<ErrorType, ReturnType>::Ok(value()(arg.value()));
                    }
                } else {
                    return Either<ErrorType, ReturnType>::Error(arg.error());
                }
            }
        }
    }

    template<typename std::enable_if<std::is_invocable_v<ValueType>>* = nullptr>
    decltype(auto) internal_apply() const {
        using ReturnType = std::invoke_result_t<ValueType>;
        if (this->isError()) {
            return Either<ErrorType, ReturnType>::Error();
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


template<typename ErrorType, typename ValueType>
std::enable_if_t<std::is_void_v<ValueType> && std::is_void_v<ErrorType>, Either<void, void>>
Ok() { return Either<void, void>::Ok(); }

template<typename ErrorType, typename ValueType>
std::enable_if_t<(!std::is_void_v<ErrorType>) && std::is_void_v<ValueType>, Either<ErrorType, void>>
Ok() { return Either<ErrorType, void>::Ok(); }

template<typename ErrorType, typename ValueType>
std::enable_if_t<!std::is_void_v<ErrorType> && !std::is_void_v<ValueType>, Either<ErrorType, ValueType>>
Ok(const ValueType& arg) { return Either<ErrorType, ValueType>::Ok(arg); }

template<typename ErrorType, typename ValueType>
std::enable_if_t<std::is_void_v<ErrorType> && !std::is_void_v<ValueType>, Either<void, ValueType>>
Ok(const ValueType& arg) { return Either<void, ValueType>::Ok(arg); }

template<typename ErrorType, typename ValueType>
std::enable_if_t<std::is_void_v<ErrorType> && std::is_void_v<ValueType>, Either<void, void>>
Error() { return Either<void, void>::Error(); }

template<typename ErrorType, typename ValueType>
std::enable_if_t<!std::is_void_v<ErrorType> && std::is_void_v<ValueType>, Either<ErrorType, void>>
Error(const ErrorType& arg) { return Either<ErrorType, void>::Error(arg); }

template<typename ErrorType, typename ValueType>
std::enable_if_t<!std::is_void_v<ErrorType> && !std::is_void_v<ValueType>, Either<ErrorType, ValueType>>
Error(const ErrorType& arg) { return Either<ErrorType, ValueType>::Error(arg); }

template<typename ErrorType, typename ValueType>
std::enable_if_t<std::is_void_v<ErrorType> && !std::is_void_v<ValueType>, Either<void, ValueType>>
Error() { return Either<void, ValueType>::Error(); }

} // namespace yafl
