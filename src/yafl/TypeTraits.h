/**
 * \brief       Yet Another Functional Library
 *
 * \copyright   2023, Ernesto Festas.
 *              Distributed under MIT license (See accompanying LICENSE file)
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

namespace {

/**
 * @ingroup TypeTraits
 * Forward declaration of Details struct
 * @tparam ...
 */
template <typename ...>
struct TraitDetails;

/**
 * @ingroup TypeTraits
 * One or more argument function traits
 * Provides function type information
 * @tparam Ret Return type
 * @tparam FirstArg First argument type
 * @tparam Args Remaining Types
 */
template <typename Ret, typename FirstArg, typename... Args>
struct TraitDetails<Ret, FirstArg, Args...> {
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
struct TraitDetails<Ret> {
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
struct Traits<std::function<Ret(&)(Args...)>> : function::TraitDetails<Ret, Args...> {
};

template<typename Ret, typename... Args>
struct Traits<std::function<Ret(Args...)>> : function::TraitDetails<Ret, Args...> {
};

template<typename Ret, typename ClassType, typename... Args>
struct Traits<std::function<Ret (ClassType::*)(Args...) const>> : function::TraitDetails<Ret, Args...> {
};

template<typename Ret, typename ClassType, typename... Args>
struct Traits<std::function<Ret (ClassType::*)(Args...)>> : function::TraitDetails<Ret, Args...> {
};

template<typename Ret, typename ClassType, typename... Args>
struct Traits<std::function<Ret (ClassType::* &)(Args...)>> : function::TraitDetails<Ret, Args...> {
};

template<typename Ret, typename... Args>
struct Traits<std::function<Ret (*)(Args...)>> : function::TraitDetails<Ret, Args...> {
};
} // namespace

/**
 * @ingroup TypeTraits
 * Template class that allows extracting information about a callable type (a function,
 * a lambda, a functor etc.).
 * It provides the callable return type, the number and types of arguments.
 * @tparam Callable Callable type to inspect
 */
template<typename Callable>
struct Details : function::Traits<decltype(std::function{std::declval<function::remove_cvref_t<Callable>>()})>{};

/**
 * Specialization for STL function
 * @tparam Ret function return type
 * @tparam Args function argument types
 */
template<typename Ret, typename ...Args>
struct Details<std::function<Ret(Args...)>> : function::Traits<std::function<Ret(Args...)>>{};


/**
 * Helper struct that enables debugging by showing the information about the type T
 * @tparam T type to show
 * Note: This will make the compile to fail but will show information for the types
 */
template<typename T>
struct WhatIsThis;

} // namespace function
} // namespace yafl
