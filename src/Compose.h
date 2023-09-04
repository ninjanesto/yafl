#pragma once

/**
 * Compose functions. Similar as piping shell commands
 * @tparam TLeft Left argument type
 * @tparam TRight Right argument type
 * @param lhs Left argument
 * @param rhs Right argument
 * @return function composed by executing rhs after lhs
 */
template <typename TLeft, typename TRight>
decltype(auto) operator|(const TLeft& lhs, const TRight& rhs) {
    return [&rhs, &lhs](auto&& ...args) {
        return rhs (lhs (args...));
    };
}