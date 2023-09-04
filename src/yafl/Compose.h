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
            return [&rhs, &lhs](auto ...args) {
                lhs(args ...);
                return rhs();
            };
        } else {
            return [&rhs, &lhs](auto ...args) {
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
    if constexpr (MonadTraits<RhsReturnType>::value) {
        return [&rhs, &lhs](auto... args) {
            return lhs(args...).bind(rhs);
        };
    } else {
        return [&rhs, &lhs](auto... args) {
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
    if constexpr (MonadTraits<LhsReturnType>::value) {
        return kleisli_compose(lhs, rhs);
    } else {
        return function_compose(lhs, rhs);
    }
}

} // namespace yafl
