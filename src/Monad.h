#pragma once

#include <memory>
#include <functional>

//mreturn :: a -> M a
//bind :: (a -> M b) -> M a -> M b
/**
 * Functional Monad class. Defines the bind function that receives a function from a -> M b and applies that function
 * to a given Monadic type, by unwrapping the value inside the monad and applying the function. Result type will depend
 * on the given function.
 * @tparam TDerivedMonad Type of monad that needs to implement the Monad contract.
 * @tparam Args Argument type for the given template type TDerivedMonad
 */
template <template <typename...> typename TDerivedMonad, typename ...Args>
class Monad {
public:
    /**
     * Binds given callable (function, function object, lambda) to the Monad value
     * @tparam Callable Callable type
     * @param callable Callback to be executed
     * @return a new Monad with the result from the application of the function
     */
    template <typename Callable>
    decltype(auto) bind(Callable&& callable) const {
        return static_cast<const TDerivedMonad<Args...>*>(this)->internal_bind(std::forward<Callable>(callable));
    }
};
