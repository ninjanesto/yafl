#pragma once

#include <memory>
#include <functional>

//pure  :: a -> A a
//apply :: A (a -> b) -> A a -> A b
/**
 * Functional Applicative Functor class. Defines the apply function that performs a partial function application
 * of the received argument to the callable object of the Applicative Functor. The apply method receives a type A(a), unwraps
 * its value and performs a partial apply to the A (a->b) function. Result type will depend on the given function.
 * @tparam TDerivedApplicative Type of Applicative Functor that needs to implement the contract.
 * @tparam Args Argument types for the given template type TDerivedApplicative
 */
template <template <typename...> typename TDerivedApplicative, typename ...Args>
class Applicative {
public:
    /**
     * Executes the internal function. It has to have been previously partially applied or the function is defined with no
     * input arguments
     * @return result of inner function execution
     */
    decltype(auto) operator()() const {
        return static_cast<const TDerivedApplicative<Args...>*>(this)->internal_apply();
    }

    /**
     * Binds given argument to the Applicative Functor value (function, function object, lambda). If the function has
     * more than one input argument then the bind is a partial application, i.e, the apply function will always
     * return a new function with an input argument less. If the function contains only one input argument then the function
     * is executed.
     * @tparam Head Type of input argument
     * @param applicative Wrapper that contains the value to be partialy applied to the Application Functor function
     * @return a new Applicative Functor containing a new function with one input argument less
     */
    template<typename Head>
    decltype(auto) operator()(const Head& head) const {
        return static_cast<const TDerivedApplicative<Args...>*>(this)->internal_apply(head);
    }

    /**
     * Binds given arguments to the Applicative Functor value (function, function object, lambda). If the function has
     * more input arguments than the ones provided, then the bind is a partial application, i.e, the apply function will always
     * return a new function with less input arguments. If the function contains exactly the same number of input
     * arguments then the function is executed.
     * @tparam Head Type of input argument
     * @tparam Tail Variadic arguments list type
     * @param applicative Wrapper that contains the value to be partialy applied to the Application Functor function
     * @return a new Applicative Functor containing a new function with one input argument less
     */
    template<typename Head, typename ...Tail>
    decltype(auto) operator()(const Head& head, const Tail& ...tail) const {
        return static_cast<const TDerivedApplicative<Args...>*>(this)->internal_apply(head)(tail...);
    }
};

template<template <typename...> typename T, typename Callable, typename ...Args>
decltype(auto) apply(const Applicative<T, Callable>& applicative, const Applicative<T, Args...>& value) {
    return applicative.apply(value);
}
