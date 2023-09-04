#pragma once

#include <memory>
#include <functional>
#include "TypeTraits.h"

namespace yafl {

//fmap :: (a -> b) -> F a -> F b
//lift :: (a -> b) -> (F a -> F b)
//fmap :: ((a->c) -> b) -> F (a->c) -> F b
/**
 * Functional Functor class. Defines the fmap function that receives a function from a -> b and applies that function
 * to a given Functor by unwrapping the value inside the functor and applying the function. Result type will depend
 * on the given function
 * @tparam TDerivedFunctor Type of functor that needs to implement the Functor contract.
 * @tparam Args Argument types for the given template type TDerivedFunctor
 */
template <template <typename...> typename TDerivedFunctor, typename ...Args>
class Functor {
public:
    /**
     * Binds given callable (function, function object, lambda) to the Functor value
     * @tparam Callable Callable type
     * @param callable Callback to be executed
     * @return a new Functor with the result from the application of the function
     */
    template <typename Callable>
    decltype(auto) fmap(Callable&& callable) const {
        return static_cast<const TDerivedFunctor<Args...>*>(this)->internal_fmap(std::forward<Callable>(callable));
    }

    /**
     * Lifts a function from a -> b and returns a new function F a -> F b, i.e, it enables a function to work on the
     * Functor abstraction level
     * @tparam Callable Callable type
     * @param callable Callback to be executed
     * @return a new function with the input argument types lifted to Functor types
     */
    template<typename Callable>
    static decltype(auto) lift(Callable&& callable) {
        return [&callable](const TDerivedFunctor<Args...>& value){
            return value.fmap(std::forward<Callable>(callable));
        };
    }
};

//fmap function
template<typename Callable, template <typename...> typename T, typename ...Args>
decltype(auto) fmap(Callable&& callable, const Functor<T, Args...>& value) {
    return value.fmap(std::forward<Callable>(callable));
}

//lift function
template<typename T, typename Callable>
decltype(auto) lift(Callable&& callable) {
    return [&callable](const T& value){
        return value.fmap(std::forward<Callable>(callable));
    };
}

template<template <typename...> typename T, typename Callable>
decltype(auto) lift(Callable&& callable) {
    using InputArgType = typename function_traits<Callable>::template ArgType<0>;

    return [&callable](const T<InputArgType>& value){
        return value.fmap(std::forward<Callable>(callable));
    };
}

} // namespace yafl
