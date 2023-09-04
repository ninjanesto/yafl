/**
 * \brief       Yet Another Functional Library
 *
 * \copyright   2023, Ernesto Festas.
 *              Distributed under MIT license (See accompanying LICENSE file)
 * \defgroup HOF High Order Functions
 */
#pragma once

#include "yafl/TypeTraits.h"
#include "yafl/Monad.h"

namespace yafl {

namespace details {
    template<typename Callable, typename Head>
    decltype(auto) uncurry_impl(Callable&& callable, Head&& value) {
        return callable(std::forward<Head>(value));
    }

    template<typename Callable, typename Head, typename ...Tail>
    decltype(auto) uncurry_impl(Callable&& callable, Head&& value, Tail&&...ts) {
        const auto result = callable(std::forward<Head>(value));
        return uncurry_impl(result, std::forward<Tail>(ts)...);
    }

    template<typename Predicate, typename ...Args>
    decltype(auto) all_true(Predicate&& predicate, Args&& ...args) {
        return (predicate(std::forward<Args>(args)) && ...);
    }

    template<typename Callable, typename Tuple, typename Head>
    decltype(auto) map_tuple_append(Callable&& callable, const Tuple& tuple, const Head& value) {
        return std::tuple_cat(tuple, std::make_tuple(callable(value)));
    }

    template<typename Callable, typename Tuple, typename Head, typename ...Tail>
    decltype(auto) map_tuple_append(Callable&& callable, const Tuple& tuple, const Head& value, Tail&&...ts) {
        const auto tp = std::tuple_cat(tuple, std::make_tuple(callable(value)));
        return map_tuple_append(std::forward<Callable>(callable), std::move(tp), std::forward<Tail>(ts)...);
    }
}

/**
 * @ingroup HOF
 * Compose functions. Similar as piping shell commands. Output of first function is
 * piped as input to the second function, except when output is void
 * @tparam TLeft Left argument type
 * @tparam TRight Right argument type
 * @param lhs Left argument
 * @param rhs Right argument
 * @return function composed by executing rhs after lhs
 */
template <typename TLeft, typename TRight>
decltype(auto) function_compose(TLeft&& lhs, TRight&& rhs) {
    using LhsReturnType = typename yafl::function::Details<TLeft>::ReturnType;
    if constexpr (yafl::function::Details<TLeft>::ArgCount > 0) {
        using InputArg = typename function::Details<TLeft>::template ArgType<0>;
        if constexpr (std::is_void_v<LhsReturnType>) {
            return [rhs = std::forward<TRight>(rhs), lhs = std::forward<TLeft>(lhs)](InputArg&& arg) {
                lhs(std::forward<InputArg>(arg));
                return rhs();
            };
        } else {
            return [rhs = std::forward<TRight>(rhs), lhs = std::forward<TLeft>(lhs)](InputArg&& arg) {
                return rhs(lhs(std::forward<InputArg>(arg)));
            };
        }
    } else {
        if constexpr (std::is_void_v<LhsReturnType>) {
            return [rhs = std::forward<TRight>(rhs), lhs = std::forward<TLeft>(lhs)]() {
                lhs();
                return rhs();
            };
        } else {
            return [rhs = std::forward<TRight>(rhs), lhs = std::forward<TLeft>(lhs)]() {
                return rhs(lhs());
            };
        }
    }
}

/**
 * @ingroup HOF
 * Compose functions that return Monadic values. Similar as piping shell commands. Output of first function is
 * piped as input to the second function, except when output is void
 * @tparam TLeft Left argument type
 * @tparam TRight Right argument type
 * @param lhs Left argument
 * @param rhs Right argument
 * @return function composed by executing rhs after lhs
 */
template <typename TLeft, typename TRight>
decltype(auto) kleisli_compose(TLeft&& lhs, TRight&& rhs) {
    using FirstArg = typename function::Details<TLeft>::template ArgType<0>;
    using RhsReturnType = typename function::Details<TRight>::ReturnType;
    static_assert(type::Details<RhsReturnType>::hasMonadicBase, "Right hand side needs to have Monadic base");

    return [rhs = std::forward<TRight>(rhs), lhs = std::forward<TLeft>(lhs)](const FirstArg& arg) -> RhsReturnType {
        using LhsReturnType = typename yafl::function::Details<TLeft>::ReturnType;
        const LhsReturnType intermediate_result = lhs(arg);

        if constexpr (std::is_void_v<typename type::Details<LhsReturnType>::ValueType>) {
            if (!intermediate_result) {
                return type::Details<RhsReturnType>::handleError(intermediate_result);
            } else {
                return rhs();
            }
        } else {
            if constexpr (type::Details<typename function::Details<TRight>::template ArgType<0>>::hasMonadicBase) {
                return rhs(intermediate_result);
            } else {
                if (!intermediate_result) {
                    return type::Details<RhsReturnType>::handleError(intermediate_result);
                }
                return rhs(intermediate_result.value());
            }
        }
    };
}

/**
 * @ingroup HOF
 * Generic composition. It can perform either function or monadic function (kleisli) composition
 * @tparam TLeft Left argument type
 * @tparam TRight Right argument type
 * @param lhs Left argument
 * @param rhs Right argument
 * @return function composed by executing rhs after lhs
 */
template <typename TLeft, typename TRight>
decltype(auto) compose(TLeft&& lhs, TRight&& rhs) {
    using LhsReturnType = typename yafl::function::Details<TLeft>::ReturnType;
    if constexpr (type::Details<LhsReturnType>::hasMonadicBase) {
        return kleisli_compose(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
    } else {
        return function_compose(std::forward<TLeft>(lhs), std::forward<TRight>(rhs));
    }
}

/**
 * @ingroup HOF
 * Curry given callable
 * @tparam Callable type of callable
 * @param callable function to execute
 * @return curried function
 */
template <typename Callable>
decltype(auto) curry(Callable&& callable) {
    if constexpr (std::is_invocable_v<Callable>) {
        return callable();
    } else {
        using FirstArg = typename function::Details<Callable>::template ArgType<0>;
        return [callable = std::forward<Callable>(callable)](const FirstArg& arg) {
            using ReturnFunType = typename function::Details<Callable>::PartialApplyFirst;

            const ReturnFunType inner_func = [callable, arg = std::move(arg)](auto&& ...args) {
                return callable(std::move(arg), std::forward<decltype(args)>(args)...);
            };

            return curry(std::move(inner_func));
        };
    }
}

/**
 * @ingroup HOF
 * Uncurry given callable
 * @tparam Callable callable type
 * @param callable callable to apply uncurry
 * @return Uncurried version of the given function
 */
template <typename Callable>
decltype(auto) uncurry(Callable&& callable) {
    return [callable = std::forward<Callable>(callable)](auto&& ...args) {
        return details::uncurry_impl(callable, std::forward<decltype(args)>(args)...);
    };
}

/**
 * @ingroup HOF
 * Partial apply given function
 * @tparam Callable type of callable
 * @tparam Args type of args
 * @param callable function to partial apply given arguments
 * @param args argument for the provided function
 * @return partially applied function
 */
template <typename Callable, typename ...Args>
decltype(auto) partial(Callable&& callable, Args&& ...args) {
    if constexpr (std::is_invocable_v<Callable, Args...>) {
        return std::invoke(callable, std::forward<Args>(args)...);
    } else {
        return [callable = std::forward<Callable>(callable), vargs = std::make_tuple(std::forward<Args>(args)...)](auto&& ...inner_args) {
            return std::apply([callable = std::move(callable)](auto&& ...apply_args){
                    return callable(std::forward<decltype(apply_args)>(apply_args)...);
                },
                std::tuple_cat(vargs, std::make_tuple(inner_args...)));
        };
    }
}

/**
 * @ingroup HOF
 * Identity function
 * @tparam Arg Type of argument
 * @param arg argument
 * @return argument
 */
template<typename Arg>
Arg id(Arg&& arg) {
    return std::forward<Arg>(arg);
}

/**
 * @ingroup HOF
 * Function that always evaluates to the first argument, ignoring its second argument.
 * @tparam First Type of argument
 * @param first argument
 * @return Returns a function that always returns the configured first argument
 */
template<typename First>
decltype(auto) constf(First&& first) {
    return [first = std::forward<First>(first)](auto&& ...) {
        return first;
    };
}

} // namespace yafl
