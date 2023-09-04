/**
 * \brief       Yet Another Functional Library
 *
 * \copyright   2023, Ernesto Festas.
 *              Distributed under MIT license (See accompanying LICENSE file)
 * \defgroup Core YAFL core
 */
#pragma once

#include <memory>
#include <functional>
#include "TypeTraits.h"

namespace yafl {
namespace core {

/**
 * @ingroup Core
 * @ingroup Functor
 * Functional Functor class. Defines the fmap function that receives a function from a -> b and applies that function
 * to a given Functor by unwrapping the value inside the functor and applying the function. Result type will depend
 * on the given function
 * @tparam TDerivedFunctor Type of functor that needs to implement the Functor contract.
 * @tparam Args Argument types for the given template type TDerivedFunctor
 */
template<template<typename...> typename TDerivedFunctor, typename ...Args>
class Functor {
public:
    /**
     * Binds given callable (function, function object, lambda) to the Functor value
     * @tparam Callable Callable type
     * @param callable Callback to be executed
     * @return a new Functor with the result from the application of the function
     */
    template<typename Callable>
    decltype(auto) fmap(Callable&& callable) const {
        return static_cast<const TDerivedFunctor<Args...> *>(this)->internal_fmap(std::forward<Callable>(callable));
    }
};

} // namespace core

namespace functor {
/**
 * @ingroup Functor
 * This function is used to apply a callable type to a value of type Functor
 * @tparam Callable callable type
 * @tparam FunctorT functor type
 * @param callable function to be applied to the given functor
 * @param functor functor to be used
 * @return Functor type containing the result of the function application
 */
template<typename Callable, typename FunctorT>
decltype(auto) fmap(Callable&& callable, const FunctorT& functor) {
    static_assert(type::DomainTypeInfo<FunctorT>::hasFunctorBase, "FunctorT argument not a Functor");
    return functor.fmap(std::forward<Callable>(callable));
}

/**
 * @ingroup Functor
 * This function is used to apply a callable type to a value of type Functor
 * In this case the provided function is lifted to work at Functor level an so,
 * this function returns a new callable that receives a Functor as argument and returns
 * a Functor
 * @tparam FunctorType Functor type
 * @tparam Callable callable type
 * @param callable function to be applied to the given functor
 * @return Function lifted to work at Functor level
 */
template<template<typename...> typename FunctorType, typename Callable>
decltype(auto) fmap(Callable&& callable) {
    if constexpr (std::is_invocable_v<Callable>) {
        return [callable = std::forward<Callable>(callable)](const FunctorType<void>& functor) {
            static_assert(type::DomainTypeInfo<FunctorType<void>>::hasFunctorBase, "Argument not a Functor");
            return functor.fmap(callable);
        };
    } else {
        using FirstArg = typename function::Info<Callable>::template ArgType<0>;

        return [callable = std::forward<Callable>(callable)](const FunctorType<FirstArg>& functor) {
            static_assert(type::DomainTypeInfo<FunctorType<FirstArg>>::hasFunctorBase, "Argument not a Functor");
            return functor.fmap(callable);
        };
    }
}
} // namespace functor

} // namespace yafl
