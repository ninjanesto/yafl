#pragma once

#include <iostream>
#include <memory>
#include <functional>
#include <variant>
#include "Functor.h"
#include "Monad.h"


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
                 //, public Applicative<Maybe>
                         , public Monad<Either, void, void> {
    friend class Functor<Either, void, void>;
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
    decltype(auto) internal_fmap(const Callable& callable) const {
        static_assert(std::is_invocable_v<Callable>, "Input argument is not invocable");
        using ReturnType = std::invoke_result_t<Callable>;
        if (this->isOk()) {
            if constexpr (std::is_void_v<ReturnType>) {
                callable();
                return Either<void, void>::Ok();
            } else {
                return Either<void, ReturnType>::Ok(callable());
            }
        } else {
            return Either<void, ReturnType>::Error();
        }
    }

    template <typename Callable>
    decltype(auto) internal_bind(const Callable& callable) const {
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
class Either<void, ValueType> : public Functor<Either, void, ValueType> {
    friend class Functor<Either, void, ValueType>;
private:
    explicit Either(bool isError) : _right{nullptr}, _isError{isError} {}
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
        if (!_isError) return *(_right.get());;
        throw std::runtime_error("Ok not defined");
    }
private:
    template <typename Callable>
    decltype(auto) internal_fmap(const Callable& callable) const {
        static_assert(std::is_invocable_v<Callable, ValueType>, "Input argument is not invocable");
        using ReturnType = std::invoke_result_t<Callable, ValueType>;
        if (this->isOk()) {
            if constexpr (std::is_void_v<ReturnType>) {
                callable(value());
                return Either<void, void>::Ok();
            } else {
                return Either<void, ReturnType>::Ok(callable(value()));
            }
        } else {
            return Either<void, ReturnType>::Error();
        }
    }

    template <typename Callable>
    decltype(auto) internal_bind(const Callable& callable) const {
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
private:
    std::unique_ptr<ValueType> _right;
    bool _isError;
};


template <typename ErrorType>
class Either<ErrorType, void> : public Functor<Either, ErrorType, void> {
    friend class Functor<Either, ErrorType, void>;
private:
    explicit Either(bool isError) : _left{nullptr}, _isError{isError} {}
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
        if (_isError) return *(_left.get());;
        throw std::runtime_error("Ok not defined");
    }
private:
    template <typename Callable>
    decltype(auto) internal_fmap(const Callable& callable) const {
        static_assert(std::is_invocable_v<Callable>, "Input argument is not invocable");
        using ReturnType = std::invoke_result_t<Callable>;
        if (this->isOk()) {
            if constexpr (std::is_void_v<ReturnType>) {
                callable();
                return Either<ErrorType, void>::Ok();
            } else {
                return Either<ErrorType, ReturnType>::Ok(callable());
            }
        } else {
            return Either<ErrorType, ReturnType>::Error(error());
        }
    }

    template <typename Callable>
    decltype(auto) internal_bind(const Callable& callable) const {
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
    std::unique_ptr<ErrorType> _left;
    bool _isError;
};

template <typename ErrorType, typename ValueType>
class Either : public Functor<Either, ErrorType, ValueType> {
    friend class Functor<Either, ErrorType, ValueType>;
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
    decltype(auto) internal_fmap(const Callable& callable) const {
        static_assert(std::is_invocable_v<Callable, ValueType>, "Input argument is not invocable");
        using ReturnType = std::invoke_result_t<Callable, ValueType>;
        if (this->isOk()) {
            if constexpr (std::is_void_v<ReturnType>) {
                callable(value());
                return Either<ErrorType, void>::Ok();
            } else {
                return Either<ErrorType, ReturnType>::Ok(callable(value()));
            }
        } else {
            return Either<ErrorType, ReturnType>::Error(error());
        }
    }

    template <typename Callable>
    decltype(auto) internal_bind(const Callable& callable) const {
        static_assert(std::is_invocable_v<Callable, ValueType>, "Input argument is not invocable");
        using ReturnType = std::invoke_result_t<Callable, ValueType>;
        using InnerTypeError = typename EitherTraits<ReturnType>::ErrorType;
        static_assert(std::is_same_v<ErrorType, InnerTypeError>, "Error type does not match");
        using InnerTypeOK = typename EitherTraits<ReturnType>::ValueType;
        if (this->isOk()) {
            return callable(this->value());
        } else {
            return Either<InnerTypeError, InnerTypeOK>::Error(this->error());
        }
    }

private:
    std::variant<ErrorType, ValueType> _value;
    bool _isError;
};

template<typename ValueType>
std::enable_if_t<!std::is_void_v<ValueType>, Either<void, ValueType>>
Ok(const ValueType& arg) { return Either<void, ValueType>::Ok(arg); }

template<typename ValueType>
std::enable_if_t<(std::is_void_v<ValueType>), Either<void, void>>
Ok() { return Either<void, void>::Ok(); }

template<typename ErrorType, typename ValueType>
std::enable_if_t<std::is_void_v<ErrorType> && std::is_void_v<ValueType>, Either<void, void>>
Ok() { return Either<void, void>::Ok(); }

template<typename ErrorType, typename ValueType>
Either<ErrorType, ValueType> Ok(const ValueType& arg) { return Either<ErrorType, ValueType>::Ok(arg); }

template<typename ErrorType>
std::enable_if_t<!std::is_void_v<ErrorType>, Either<ErrorType, void>>
Error(const ErrorType& arg) { return Either<ErrorType, void>::Error(arg); }

template<typename ErrorType>
std::enable_if_t<(std::is_void_v<ErrorType>), Either<void, void>>
Error() { return Either<void, void>::Error(); }

template<typename ErrorType, typename ValueType>
std::enable_if_t<std::is_void_v<ErrorType> && std::is_void_v<ValueType>, Either<void, void>>
Error() { return Either<void, void>::Error(); }

template<typename ErrorType, typename R>
Either<ErrorType, R> Error(const ErrorType& arg) { return Either<ErrorType, R>::Error(arg); }
