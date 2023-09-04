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


/**
 * @ingroup Core
 * Structure to validate if given type is derived from Functor. This class works as sinkhole for all
 * non functor based types and sets a value attribute as false
 *
 * @tparam T type to validate
 */
template<typename T>
struct IsFunctorBase {
    ///boolean flag that states whether type T is a functor or not
    static constexpr bool value = false;
};

/**
 * @ingroup Core
 * Structure to validate if given type is derived from Functor. This class validates if the given type
 * is derived from Functor and set the value attribute accordingly
 * @tparam FunctorType Functor type
 * @tparam Args functor type arguments
 */
template<template<typename...> typename FunctorType, typename ...Args>
struct IsFunctorBase<FunctorType<Args...>> {
    /// Base type
    using BaseType = Functor<FunctorType, Args...>;
    /// Derived type
    using DerivedType = FunctorType<Args...>;
    ///boolean flag that states whether type T is a functor or not
    static constexpr bool value = std::is_base_of_v<BaseType, DerivedType>;
};

} // namespace core

/**
 * This function is used to apply a callable type to a value of type Functor
 * @tparam Callable callable type
 * @tparam FunctorT functor type
 * @param callable function to be applied to the given functor
 * @param functor functor to be used
 * @return Functor type containing the result of the function application
 */
template<typename Callable, typename FunctorT>
decltype(auto) fmap(Callable&& callable, const FunctorT& functor) {
    static_assert(core::IsFunctorBase<FunctorT>::value, "FunctorT argument not a Functor");
    return functor.fmap(std::forward<Callable>(callable));
}

/**
 * This function is used to apply a callable type to a value of type Functor
 * In this case the provided function is lifted to work at Functor level an so,
 * this function returns a new callable that receives a Functor as argument and returns
 * a Functor
 * @tparam Callable callable type
 * @param callable function to be applied to the given functor
 * @return Function lifted to work at Functor level
 */
template<typename Callable>
decltype(auto) fmap(Callable&& callable) {
    return [callable = std::forward<Callable>(callable)](const auto& functor){
        return functor.fmap(callable);
    };
}

} // namespace yafl
