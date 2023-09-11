/**
 * \brief       Group of High Order Functions that provide features like function composition,
 * partial application, currying and uncurrying, etc
 *
 * \copyright   2023, Ernesto Festas.
 *              Distributed under MIT license (See accompanying LICENSE file)
 * \defgroup    HOF High Order Functions
 */
#pragma once

#include "yafl/TypeTraits.h"
#include "yafl/Monad.h"

namespace yafl {

/**
 * @ingroup HOF
 * Function that applies given predicate to all input arguments and returns true
 * if ALL of the arguments satisfies the given predicate
 * @tparam Predicate Predicate function type
 * @tparam Args Input arguments types
 * @param predicate predicate function
 * @param args input arguments
 * @return true if all arguments verify the predicate and false otherwise
 */
template<typename Predicate, typename ...Args>
decltype(auto) all(Predicate&& predicate, Args&& ...args) {
    return (predicate(std::forward<Args>(args)) && ...);
}

/**
 * @ingroup HOF
 * Function that applies given predicate to all input arguments and returns true
 * if ANY of the arguments satisfies the given predicate
 * @tparam Predicate Predicate function type
 * @tparam Args Input arguments types
 * @param predicate predicate function
 * @param args input arguments
 * @return true if any arguments verify the predicate and false otherwise
 */
template<typename Predicate, typename ...Args>
decltype(auto) any(Predicate&& predicate, Args&& ...args) {
    return (predicate(std::forward<Args>(args)) || ...);
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
    using LhsReturnType = typename function::Info<TLeft>::ReturnType;
    if constexpr (std::is_invocable_v<TLeft>) {
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
    } else {
        using InputArg = typename function::Info<TLeft>::template ArgType<0>;
        if constexpr (std::is_void_v<LhsReturnType>) {
            return [rhs = std::forward<TRight>(rhs), lhs = std::forward<TLeft>(lhs)](InputArg&& arg) {
                lhs(std::move(arg));
                return rhs();
            };
        } else {
            return [rhs = std::forward<TRight>(rhs), lhs = std::forward<TLeft>(lhs)](InputArg&& arg) {
                return rhs(lhs(std::move(arg)));
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
    using FirstArg = typename function::Info<TLeft>::template ArgType<0>;
    using RhsReturnType = typename function::Info<TRight>::ReturnType;
    static_assert(type::DomainTypeInfo<RhsReturnType>::hasMonadicBase, "Right hand side needs to have Monadic base");

    return [rhs = std::forward<TRight>(rhs), lhs = std::forward<TLeft>(lhs)](const FirstArg& arg) -> RhsReturnType {
        using LhsReturnType = typename function::Info<TLeft>::ReturnType;
        const LhsReturnType intermediate_result = lhs(arg);

        if constexpr (std::is_void_v<typename type::DomainTypeInfo<LhsReturnType>::ValueType>) {
            if (!intermediate_result) {
                return type::DomainTypeInfo<RhsReturnType>::handleError(intermediate_result);
            } else {
                return rhs();
            }
        } else {
            if constexpr (type::DomainTypeInfo<typename function::Info<TRight>::template ArgType<0>>::hasMonadicBase) {
                return rhs(intermediate_result);
            } else {
                if (!intermediate_result) {
                    return type::DomainTypeInfo<RhsReturnType>::handleError(intermediate_result);
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
    using LhsReturnType = typename function::Info<TLeft>::ReturnType;
    if constexpr (type::DomainTypeInfo<LhsReturnType>::hasMonadicBase) {
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
    if constexpr (std::is_invocable_v<std::decay_t<Callable>>) {
        return callable();
    } else {
        using FirstArg = typename function::Info<Callable>::template ArgType<0>;
        return [callable = std::forward<Callable>(callable)](const FirstArg& arg) {
            using ReturnFunType = typename function::Info<Callable>::PartialApplyFirst;

            const ReturnFunType inner_func = [callable, arg = std::move(arg)](auto&& ...args) {
                return callable(std::move(arg), std::forward<decltype(args)>(args)...);
            };

            return curry(std::move(inner_func));
        };
    }
}

namespace {
    template<typename Callable, typename Head>
    decltype(auto) uncurry_impl(Callable&& callable, Head&& value) {
        return callable(std::forward<Head>(value));
    }

    template<typename Callable, typename Head, typename ...Tail>
    decltype(auto) uncurry_impl(Callable&& callable, Head&& value, Tail&&...ts) {
        const auto result = callable(std::forward<Head>(value));
        return uncurry_impl(result, std::forward<Tail>(ts)...);
    }
} // end namespace

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
        return uncurry_impl(callable, std::forward<decltype(args)>(args)...);
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
    using IsTupleSubset = tuple::IsTupleSubset<typename function::Info<Callable>::ArgTypes, std::tuple<Args...>>;
    static_assert(IsTupleSubset::value, "Input arguments are not a subset of the Callable arguments");

    if constexpr (std::is_invocable_v<std::decay_t<Callable>, std::decay_t<Args>...>) {
        return std::invoke(callable, std::forward<Args>(args)...);
    } else {

        using RemainingArgsTuple = tuple::TupleSubset<IsTupleSubset::index, typename function::Info<Callable>::ArgTypes>;
        using ReturnType = function::FunctionFromTuple<typename function::Info<Callable>::ReturnType, RemainingArgsTuple>;

        const ReturnType f = [callable = std::forward<Callable>(callable), vargs = std::make_tuple(std::forward<Args>(args)...)](auto&& ...inner_args) {
            return std::apply([callable = std::move(callable)](auto&& ...apply_args){
                                  return callable(std::forward<decltype(apply_args)>(apply_args)...);
                              },
                              std::tuple_cat(vargs, std::make_tuple(inner_args...)));
        };

        return f;
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
