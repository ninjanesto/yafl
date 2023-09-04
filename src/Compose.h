#pragma once

namespace yafl {

/**
 * Compose functions. Similar as piping shell commands
 * @tparam TLeft Left argument type
 * @tparam TRight Right argument type
 * @param lhs Left argument
 * @param rhs Right argument
 * @return function composed by executing rhs after lhs
 */

#include <utility>

template <typename TLeft, typename TRight, typename ...Args>
decltype(auto) operator|(const TLeft& lhs, const TRight& rhs) {
    return [&rhs, &lhs](Args&& ...args) {
        return rhs (lhs (std::forward<Args>(args)...));
    };
}

} // namespace yafl
