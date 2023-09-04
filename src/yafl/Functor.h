/**
 * \brief       Yet Another Functional Library
 *
 * \copyright   Critical TechWorks SA
 * \defgroup Core YAFL core
 */
#pragma once

#include <memory>
#include <functional>
#include "TypeTraits.h"

namespace yafl::core {

/**
 * @ingroup Core
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
};


} // namespace yafl::core
