#pragma once

#include "yafl/TypeTraits.h"

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
decltype(auto) compose(const TLeft& lhs, const TRight& rhs) {
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

} // namespace yafl

template <typename TLeft, typename TRight>
decltype(auto) operator|(const TLeft& lhs, const TRight& rhs) {
    return yafl::compose(lhs, rhs);
}
