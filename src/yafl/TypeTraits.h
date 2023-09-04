/**
 * \brief       Yet Another Functional Library
 *
 * \copyright   Critical TechWorks SA
 * \defgroup TypeTraits Function type traits
 */
#pragma once

#include <type_traits>
#include <cstddef>
#include <functional>

namespace yafl {
namespace function {

/**
 * @ingroup TypeTraits
 * Remove const volatile reference from input argument type T
 * @tparam T
 */
template<typename T>
using remove_cvref_t = typename std::remove_cv_t<std::remove_reference_t<T>>;

/**
 * @ingroup TypeTraits
 * Forward declaration of Details struct
 * @tparam ...
 */
template <typename ...>
struct Details;

/**
 * @ingroup TypeTraits
 * One or more argument function traits
 * Provides function type information
 * @tparam Ret Return type
 * @tparam FirstArg First argument type
 * @tparam Args Remaining Types
 */
template <typename Ret, typename FirstArg, typename... Args>
struct Details<Ret, FirstArg, Args...> {
    ///Function Signature
    using Signature = std::function<Ret(FirstArg, Args...)>;
    ///Function Signature after applying first argument
    using PartialApplyFirst = std::function<Ret(Args...)>;
    ///Return type
    using ReturnType = Ret;
    ///Function argument types, provided in a tuple
    using ArgTypes = std::tuple<remove_cvref_t<FirstArg>, remove_cvref_t<Args>...>;
    ///Number of arguments
    static constexpr std::size_t ArgCount = sizeof...(Args) + 1;
    ///Getter for argument with ID idx
    template <std::size_t idx>
    using ArgType = std::tuple_element_t<idx, ArgTypes>;
};

/**
 * @ingroup TypeTraits
 * Zero arguments function traits
 * Provides function type information
 * @tparam Ret Return type
 */
template <typename Ret>
struct Details<Ret> {
    ///Function Signature
    using Signature = std::function<Ret()>;
    ///Return type
    using ReturnType = Ret;
    ///Function argument types, provided in a tuple
    using ArgTypes = std::tuple<>;
    ///Number of arguments
    static constexpr std::size_t ArgCount = 0;
};

template<typename>
struct Traits;

template<typename Ret, typename... Args>
struct Traits<std::function<Ret(&)(Args...)>> : function::Details<Ret, Args...> {};

template<typename Ret, typename... Args>
struct Traits<std::function<Ret(Args...)>> : function::Details<Ret, Args...> {};

template<typename Ret, typename ClassType, typename... Args>
struct Traits<std::function<Ret (ClassType::*)(Args...) const>> : function::Details<Ret, Args...> {};

template<typename Ret, typename ClassType, typename... Args>
struct Traits<std::function<Ret (ClassType::*)(Args...)>> : function::Details<Ret, Args...> {};

template<typename Ret, typename ClassType, typename... Args>
struct Traits<std::function<Ret (ClassType::* &)(Args...)>> : function::Details<Ret, Args...> {};

template<typename Ret, typename... Args>
struct Traits<std::function<Ret (*)(Args...)>> : function::Details<Ret, Args...> {};

} // namespace function

/**
 * @ingroup TypeTraits
 * Template class that allows extracting information about a callable type (a function,
 * a lambda, a functor etc.).
 * It provides the callable return type, the number and types of arguments.
 * @tparam Callable Callable type to inspect
 */
template<typename Callable>
struct FunctionTraits : function::Traits<decltype(std::function{std::declval<Callable>()})>{};

} // namespace yafl
