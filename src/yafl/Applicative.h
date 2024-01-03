/**
 * \brief       YAFL core contains definitions for Functor, Applicative Functor and Monad
 *
 * \copyright   2023, Ernesto Festas.
 *              Distributed under MIT license (See accompanying LICENSE file)
 * \defgroup    Core YAFL core
 * \defgroup    Applicative Applicative Functor (part of YAFL core)
 */
#pragma once

#include <memory>
#include <functional>

namespace yafl {
namespace core {

/**
 * @ingroup Core
 * @ingroup Applicative
 *
 * Functional Applicative Functor class. Defines the apply function that performs a partial function application
 * of the received argument to the callable object of the Applicative Functor. The apply method receives a type A(a), unwraps
 * its value and performs a partial apply to the A (a->b) function. Result type will depend on the given function.
 * @tparam TDerivedApplicative Type of Applicative Functor that needs to implement the contract.
 * @tparam Args Argument types for the given template type TDerivedApplicative
 */
template<template<typename...> typename TDerivedApplicative, typename ...Args>
class Applicative {
public:
    /**
     * Executes the internal function. It has to have been previously partially applied or the function is defined with no
     * input arguments
     * @return result of inner function execution
     */
    decltype(auto) operator()() const {
        return static_cast<const TDerivedApplicative<Args...> *>(this)->internal_apply();
    }

    /**
     * Binds given argument to the Applicative Functor value (function, function object, lambda). If the function has
     * more than one input argument then the bind is a partial application, i.e, the apply function will always
     * return a new function with an input argument less. If the function contains only one input argument then the function
     * is executed.
     * @tparam Head Type of input argument
     * @param head Wrapper that contains the value to be partialy applied to the Application Functor function
     * @return a new Applicative Functor containing a new function with one input argument less
     */
    template<typename Head>
    decltype(auto) operator()(Head&& head) const {
        return static_cast<const TDerivedApplicative<Args...> *>(this)->internal_apply(std::forward<Head>(head));
    }

    /**
     * Binds given arguments to the Applicative Functor value (function, function object, lambda). If the function has
     * more input arguments than the ones provided, then the bind is a partial application, i.e, the apply function will always
     * return a new function with less input arguments. If the function contains exactly the same number of input
     * arguments then the function is executed.
     * @tparam Head Type of input argument
     * @tparam Tail Variadic arguments list type
     * @param head Wrapper that contains the value to be partialy applied to the Application Functor function
     * @param tail Remaining arguments
     * @return a new Applicative Functor containing a new function with one input argument less
     */
    template<typename Head, typename ...Tail>
    decltype(auto) operator()(Head&& head, Tail&&...tail) const {
        return static_cast<const TDerivedApplicative<Args...> *>(this)->internal_apply(std::forward<Head>(head))(std::forward<Tail>(tail)...);
    }
};

} // namespace core

namespace applicative {

/**
 * @ingroup Applicative
 *
 * This function is used to apply a callable type to a value of type Applicative.
 * Applicative Functors are used to apply sequential invocation (similar to partial invocation).
 * @tparam ApplicableT callable type
 * @tparam ApplicableArg applicative argument type
 * @param callable function to be applied to the given applicative
 * @param applicative applicative to be used
 * @return Applicative Functor type containing the result of the function application
 */
template<typename ApplicableT, typename ApplicableArg>
decltype(auto) apply(ApplicableT&& callable, const ApplicableArg& applicative) {
    static_assert(type::DomainTypeInfo<ApplicableT>::hasApplicativeBase, "ApplicableT argument not a Applicative");
    return callable(applicative);
}

/**
 * @ingroup Applicative
 *
 * This function is used to apply a callable type to a value of type Applicative.
 * Applicative Functors are used to apply sequential invocation (similar to partial invocation)
 * @tparam ApplicableT callable type
 * @tparam ApplicableArg applicative argument type
 * @param callable function to be applied to the given applicative
 * @return Applicative Functor type containing the result of the function application
 */
template<typename ApplicableT>
decltype(auto) apply(ApplicableT&& callable) {
    return [callable = std::forward<ApplicableT>(callable)](auto&& ...args) {
        static_assert(type::DomainTypeInfo<ApplicableT>::hasApplicativeBase, "ApplicableT argument not a Applicative");
        return callable(std::forward<decltype(args)>(args)...);
    };
}

} // namespace applicative
} // namespace yafl
