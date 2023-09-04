/**
 * \file
 * \brief       Yet Another Functional Library
 *
 * \project     Critical TechWorks SA
 * \copyright   Critical TechWorks SA
 */
#pragma once

#include "yafl/TypeTraits.h"
#include "yafl/Monad.h"

namespace yafl {

/**
 * Compose functions. Similar as piping shell commands. Output of first function is
 * piped as input to the second function, except when output is void
 * @tparam TLeft Left argument type
 * @tparam TRight Right argument type
 * @param lhs Left argument
 * @param rhs Right argument
 * @return function composed by executing rhs after lhs
 */
template <typename TLeft, typename TRight>
decltype(auto) function_compose(const TLeft& lhs, const TRight& rhs) {
    using LhsReturnType = typename yafl::function_traits<TLeft>::ReturnType;
    if constexpr (yafl::function_traits<TLeft>::ArgCount > 0) {
        if constexpr (std::is_void_v<LhsReturnType>) {
            return [&rhs, &lhs](auto&& ...args) {
                lhs(args ...);
                return rhs();
            };
        } else {
            return [&rhs, &lhs](auto&& ...args) {
                return rhs(lhs(args ...));
            };
        }
    } else {
        if constexpr (std::is_void_v<LhsReturnType>) {
            return [&rhs, &lhs]() {
                lhs();
                return rhs();
            };
        } else {
            return [&rhs, &lhs]() {
                return rhs(lhs());
            };
        }
    }
}

/**
 * Compose functions that return Monadic values. Similar as piping shell commands. Output of first function is
 * piped as input to the second function, except when output is void
 * @tparam TLeft Left argument type
 * @tparam TRight Right argument type
 * @param lhs Left argument
 * @param rhs Right argument
 * @return function composed by executing rhs after lhs
 */
template <typename TLeft, typename TRight>
decltype(auto) kleisli_compose(const TLeft& lhs, const TRight& rhs) {
    using RhsReturnType = typename yafl::function_traits<TRight>::ReturnType;
    if constexpr (IsMonadBase<RhsReturnType>::value) {
        return [&rhs, &lhs](auto&&... args) {
            return lhs(args...).bind(rhs);
        };
    } else {
        return [&rhs, &lhs](auto&&... args) {
            return lhs(args...).fmap(rhs);
        };
    }
}

/**
 * Generic composition. It can perform either function or monadic function (kleisli) composition
 * @tparam TLeft Left argument type
 * @tparam TRight Right argument type
 * @param lhs Left argument
 * @param rhs Right argument
 * @return function composed by executing rhs after lhs
 */
template <typename TLeft, typename TRight>
decltype(auto) compose(const TLeft& lhs, const TRight& rhs) {
    using LhsReturnType = typename yafl::function_traits<TLeft>::ReturnType;
    if constexpr (IsMonadBase<LhsReturnType>::value) {
        return kleisli_compose(lhs, rhs);
    } else {
        return function_compose(lhs, rhs);
    }
}

/**
 * Curry given callable
 * @tparam F type of callable
 * @param f function to execute
 * @return curried function
 */
template <typename F>
constexpr decltype(auto) curry(const F& f) {
    if constexpr (std::is_invocable<F>()) {
        return f();
    } else {
        using ReturnFunType = typename function_traits<F>::PartialApplyFirst;
        using FirstArg = typename function_traits<F>::template ArgType<0>;

        return [f= std::move(f)](FirstArg&& arg) {
            const ReturnFunType inner_func = [f, arg = std::move(arg)](auto&& ...args) {
                return std::apply(f, std::tuple_cat(std::make_tuple(arg), std::make_tuple(args...)));
            };

            return curry(std::move(inner_func));
        };
    }
}

/**
 * Partial apply given function
 * @tparam F type of callable
 * @tparam Args type of args
 * @param f function to partial apply given arguments
 * @param args argument for the provided function
 * @return partially applied function
 */
template <typename F, typename ...Args>
constexpr decltype(auto) partial(F&& f, Args&& ...args) {
    if constexpr (std::is_invocable<F>{}) {
        return f();
    } else {
        return [f=std::forward<F>(f), vargs = std::make_tuple(std::forward<Args>(args) ...)](auto&& ...inner_args) {
            return std::apply(f, std::tuple_cat(vargs, std::make_tuple(inner_args...)));
        };
    }
}

/**
 * Identity function
 * @tparam Arg Type of argument
 * @param arg argument
 * @return argument
 */
template<typename Arg>
constexpr auto id(Arg&& arg) noexcept {
    return std::forward<Arg>(arg);
}

} // namespace yafl
