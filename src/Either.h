#pragma once

#include <iostream>
#include <memory>
#include <functional>
#include <variant>
#include "Functor.h"

// Either Monad
//template <typename L, typename R>
//class Either;

template<typename ...T>
class Either;

template<>
class Either<> : public Functor<Either> {
                 //public Applicative<Maybe>,
                 //public Monad<Maybe> {
    friend class Functor<Either>;
private:
    explicit Either(bool v) : _value(v) {}
public:
    static Either<> Left() {
        return Either<>(false);
    }

    static Either<> Right() {
        return Either<>{true};
    }

    [[nodiscard]] bool isLeft() const { return !_value; }

    [[nodiscard]] bool isRight() const { return _value; }

    void left() const {
        if (!_value) return;
        throw std::runtime_error("Left not defined");
    }

    void right() const {
        if (_value) return;
        throw std::runtime_error("Right not defined");
    }

private:
    template <typename Callable>
    decltype(auto) internal_fmap(const Callable& callable) const {
        static_assert(std::is_invocable_v<Callable>, "Input argument in not invocable");
        using ReturnType = std::invoke_result_t<Callable>;
        if (this->isRight()) {
            if constexpr (std::is_void_v<ReturnType>) {
                callable();
                return Either<>::Right();
            } else {
                return Either<void, ReturnType>::Right(callable());
            }
        } else {
            if constexpr (std::is_void_v<ReturnType>) {
                return Either<>::Left();
            } else {
                return Either<void, ReturnType>::Left();
            }
        }
    }
private:
    bool _value;
};

template <typename R>
class Either<void, R> : public Functor<Either, void, R> {
    friend class Functor<Either, void, R>;
private:
    explicit Either(bool isLeft) : _isLeft{isLeft}, _right{nullptr} {}
public:
    static Either<void, R> Left() {
        return Either<void, R>(true);
    }

    static Either<void, R> Right(const R& value) {
        Either<void, R> result(false);
        result._right = std::make_unique<R>(value);
        return result;
    }

    [[nodiscard]] bool isLeft() const { return _isLeft; }

    [[nodiscard]] bool isRight() const { return !_isLeft; }

    void left() const {
        if (_isLeft) return;
        throw std::runtime_error("Left not defined");
    }

    R right() const {
        if (!_isLeft) return *(_right.get());;
        throw std::runtime_error("Right not defined");
    }
private:
    template <typename Callable>
    decltype(auto) internal_fmap(const Callable& callable) const {
        static_assert(std::is_invocable_v<Callable, R>, "Input argument in not invocable");
        using ReturnType = std::invoke_result_t<Callable, R>;
        if (this->isRight()) {
            if constexpr (std::is_void_v<ReturnType>) {
                callable(right());
                return Either<>::Right();
            } else {
                return Either<void, ReturnType>::Right(callable(right()));
            }
        } else {
            if constexpr (std::is_void_v<ReturnType>) {
                return Either<>::Left();
            } else {
                return Either<void, ReturnType>::Left();
            }
        }
    }
private:
    std::unique_ptr<R> _right;
    bool _isLeft;
};
//
//template <typename L, typename R, typename std::enable_if_t<std::is_void_v<L>>
//class Either<void, R> : public Functor<Either, R> {
//    friend class Functor<Either, R>;
//    class LeftType{};
//    class RightType{};
//private:
//    inline static auto leftType = LeftType{};
//    inline static auto rightType = RightType{};
//    explicit Either(const L& value, const LeftType&)
//            : _left{std::make_unique<L>(value)}
//            , _isLeft{true}{}
//    explicit Either(const RightType&)
//            : _left{nullptr}
//            , _isLeft{false}{}
//public:
//    static Either<void, R> Left(const L& value) {
//        return Either<L, void>(value, leftType);
//    }
//
//    static Either<L, void> Right() {
//        return Either<L, void>(rightType);
//    }
//
//    [[nodiscard]] bool isLeft() const { return _isLeft; }
//
//    [[nodiscard]] bool isRight() const { return !_isLeft; }
//
//    L left() const {
//        if (_isLeft) return *(_left.get());
//        throw std::runtime_error("Left not defined");
//    }
//
//    void right() const {
//        if (!_isLeft) return;
//        throw std::runtime_error("Right not defined");
//    }
//private:
//    template <typename Callable>
//    decltype(auto) internal_fmap(const Callable& callable) const {
//        static_assert(std::is_invocable_v<Callable>, "Input argument in not invocable");
//        using ReturnType = std::invoke_result_t<Callable>;
//        if (this->isRight()) {
//            if constexpr (std::is_void_v<ReturnType>) {
//                callable();
//                return Either<L, void>::Right();
//            } else {
//                return Either<L, ReturnType>::Right(callable());
//            }
//        } else {
//            if constexpr (std::is_void_v<ReturnType>) {
//                return Either<L, void>::Left(left());
//            } else {
//                return Either<L, ReturnType>::Left(left());
//            }
//        }
//    }
//
//private:
//    std::unique_ptr<L> _left;
//    bool _isLeft;
//};

template <typename L, typename R>
class Either<L, R> : public Functor<Either, L, R> {
    friend class Functor<Either, L, R>;
private:
    explicit Either(bool isLeft): _isLeft{isLeft}{}
public:
    static Either<L, R> Left(const L& value) {
        Either<L, R> result{true};
        result._value.template emplace<0>(value);
        return result;
    }

    static Either<L,R> Right(const R& value) {
        Either<L, R> result{false};
        result._value.template emplace<1>(value);
        return result;
    }

    [[nodiscard]] bool isLeft() const { return _isLeft; }

    [[nodiscard]] bool isRight() const { return !_isLeft; }

    L left() const {
        if (isLeft()) return std::get<0>(_value);
        throw std::runtime_error("Left not defined");
    }

    R right() const {
        if (isRight()) return std::get<1>(_value);
        throw std::runtime_error("Right not defined");
    }
private:
    template <typename Callable>
    decltype(auto) internal_fmap(const Callable& callable) const {
        static_assert(std::is_invocable_v<Callable, R>, "Input argument in not invocable");
        using ReturnType = std::invoke_result_t<Callable, R>;
        if (this->isRight()) {
            if constexpr (std::is_void_v<ReturnType>) {
                callable(right());
                return Either<L, void>::Right();
            } else {
                return Either<L, ReturnType>::Right(callable(right()));
            }
        } else {
            if constexpr (std::is_void_v<ReturnType>) {
                return Either<L, void>::Left(left());
            } else {
                return Either<L, ReturnType>::Left(left());
            }
        }
    }

private:
    std::variant<L, R> _value;
    bool _isLeft;
};


template<typename ...T>
Either<T...> Right(T&& ...args);

template<>
Either<> Right() { return Either<>::Right(); }

template<typename R>
Either<void, R> Right(const R& arg) { return Either<void, R>::Right(arg); }

template<typename L, typename R>
Either<L, R> Right(const R& arg) { return Either<L, R>::Right(arg); }

template<typename ...T>
Either<T...> Left(T&& ...args);

template<>
Either<> Left() { return Either<>::Left(); }

template<typename R>
Either<void, R> Left() { return Either<void, R>::Left(); }

template<typename L, typename R>
Either<L, R> Left(const L& arg) { return Either<L, R>::Left(arg); }



//template<typename L = std::monostate, typename R = std::monostate>
//Either<L, R> Left(const L& value) { return Either<L, R>::createLeft(value); }
//
//template<typename L = std::monostate, typename R = std::monostate>
//Either<L, R> Right(const R& value) { return Either<L, R>::createRight(value); }
//
//template<typename L = std::monostate, typename R = std::monostate>
//class Either : public Functor<Either, L, R>,
//               public Monad<Either, L, R> {
//    friend class Functor<Either, L, R>;
//    friend class Monad<Either, L, R>;
//
//private:
//    std::variant<L, R> _value;
//    Either() = default;
//public:
//    static Either<L,R> createLeft(const L& value) {
//        Either<L,R> result;
//        result._value.template emplace<0>(value);
//        return result;
//    }
//
//    static Either<L,R> createRight(const R& value) {
//        Either<L,R> result;
//        result._value.template emplace<1>(value);
//        return result;
//    }
//
//    [[nodiscard]] bool isLeft() const {
//        return _value.index() == 0;
//    }
//
//    [[nodiscard]] bool isRight() const {
//        return _value.index() == 1;
//    }
//
//    [[nodiscard]] L left() const {
//        return std::get<0>(_value);
//    }
//
//    [[nodiscard]] R right() const {
//        return std::get<1>(_value);
//    }
//
//    [[nodiscard]] L leftOr(const L& defaultValue) const {
//        return (isLeft()) ? left() : defaultValue;
//    }
//
//    [[nodiscard]] R rightOr(const R& defaultValue) const {
//        return (isRight()) ? right() : defaultValue;
//    }
//
//private:
//    template <typename F>
//    decltype(auto) internal_bind(F&& fa) const {
//        using rtype = decltype(fa(this->right()));
//        return this->isRight() ? fa(this->right())
//                               : Left<L, decltype(std::declval<rtype>().right())>(this->left());
//    }
//
//    template <typename F>
//    decltype(auto) internal_fmap(F&& f) const {
//        return this->isRight() ? Right<L, decltype(f(this->right()))>(f(this->right()))
//                               : Left<L, decltype(f(this->right()))>(this->left());
//    }
//};
