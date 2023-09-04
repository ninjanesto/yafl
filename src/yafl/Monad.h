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

namespace yafl {
namespace core {

/**
 * @ingroup Core
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

/**
 * @ingroup Core
 * Structure to validate if given type is derived from Monad. This class works as sinkhole for all
 * non monadic types and sets a value attribute as false
 *
 * @tparam T type to validate
 */
template <typename T>
struct IsMonadicBase {
    ///boolean flag that states whether type T is a monad or not
    static constexpr bool value = false;
};

/**
 * @ingroup Core
 * Structure to validate if given type is derived from Monad. This class validates if the given type
 * is derived from Monad and set the value attribute accordingly
 * @tparam MonadType monad type
 * @tparam Args monad type arguments
 */
template <template <typename...> typename MonadType, typename ...Args>
struct IsMonadicBase<MonadType<Args...>> {
    /// Base type
    using BaseType = Monad<MonadType, Args...>;
    /// Derived type
    using DerivedType = MonadType<Args...>;
    ///boolean flag that states whether type T is a monad or not
    static constexpr bool value = std::is_base_of_v<BaseType, DerivedType>;
};
} // namespace core

/**
 * This function is used to bind (flatmap) a callable type to a value of type Monad
 * @tparam Callable callable type
 * @tparam MonadT monad type
 * @param callable function to be applied to the given monad
 * @param monad monad to be used
 * @return Monad type containing the result of the function application
 */
template<typename Callable, typename MonadT>
decltype(auto) bind(Callable&& callable, const MonadT& monad) {
    static_assert(core::IsMonadicBase<MonadT>::value, "MonadT argument not a Monad");
    return monad.bind(std::forward<Callable>(callable));
}

/**
 * This function is used to bind (flatmap) a callable type to a value of type Monad
 * In this case the provided function is lifted to work at Monad level an so,
 * this function returns a new callable that receives a Monad as argument and returns
 * a Monad
 * @tparam Callable callable type
 * @param callable function to be applied to the given functor
 * @return Function lifted to work at Monad level
 */
template<typename Callable>
decltype(auto) bind(Callable&& callable) {
    return [callable = std::forward<Callable>(callable)](const auto& monad){
        return monad.bind(callable);
    };
}

} // namespace yafl::core
