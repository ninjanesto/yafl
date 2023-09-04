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
 * @ingroup Function
 * Remove const volatile reference from input argument type T
 * @tparam T
 */
template<typename T>
using remove_cvref_t = typename std::remove_cv_t<std::remove_reference_t<T>>;

namespace {

/**
 * @ingroup FunctionDetails
 * Forward declaration of Details struct
 * @tparam ...
 */
template<typename ...>
struct TraitDetails;

/**
 * @ingroup FunctionDetails
 * One or more argument function traits
 * Provides function type information
 * @tparam Ret Return type
 * @tparam FirstArg First argument type
 * @tparam Args Remaining Types
 */
template<typename Ret, typename FirstArg, typename... Args>
struct TraitDetails<Ret, FirstArg, Args...> {
    ///Function Signature
    using Signature = std::function<Ret(FirstArg, Args...)>;
    ///Function Signature lifted to TLift
    template<template<typename...> typename TLift>
    using LiftedSignature = std::function<TLift<Ret>(const TLift<remove_cvref_t<FirstArg>>&, const TLift<remove_cvref_t<Args>>&...)>;
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
 * @ingroup FunctionDetails
 * Zero arguments function traits
 * Provides function type information
 * @tparam Ret Return type
 */
template<typename Ret>
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
 * @ingroup Function
 * Template class that allows extracting information about a callable type (a function,
 * a lambda, a functor etc.).
 * It provides the callable return type, the number and types of arguments.
 * @tparam Callable Callable type to inspect
 */
template<typename Callable>
struct Details : function::Traits<decltype(std::function{std::declval<function::remove_cvref_t<Callable>>()})> {};

/**
 * @ingroup Function
 * Specialization for STL function
 * @tparam Ret function return type
 * @tparam Args function argument types
 */
template<typename Ret, typename ...Args>
struct Details<std::function<Ret(Args...)>> : function::Traits<std::function<Ret(Args...)>> {};

} // namespace function

namespace type {

/**
 * @ingroup Type
 * Helper struct that enables debugging by showing the information about the type T
 * @tparam T type to show
 * Note: This will make the compile to fail but will show information for the types
 */
template<typename T>
struct WhatIsThis;

/**
 * @ingroup Type
 * Yafl Type traits
 */
template<typename>
struct Details {
    ///boolean flag that states whether type T is a Functor or not
    static constexpr bool hasFunctorBase = false;
    ///boolean flag that states whether type T is a Applicative or not
    static constexpr bool hasApplicativeBase = false;
    ///boolean flag that states whether type T is a Monad or not
    static constexpr bool hasMonadicBase = false;
};

/**
 * @ingroup Type
 * Helper class that introspects whether a type T is a Callable or not
 * @tparam T Type to check
 */
template <typename T>
struct IsCallable {
private:
    template <typename U>
    static auto test(int) -> decltype(void(std::declval<U>()()), std::true_type{});

    template <typename>
    static auto test(...) -> std::false_type;

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

} // namespace type
} // namespace yafl
