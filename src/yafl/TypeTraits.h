/**
 * \file
 * \brief       Yet Another Functional Library
 *
 * \project     Critical TechWorks SA
 * \copyright   Critical TechWorks SA
 */
#pragma once

#include <type_traits>
#include <cstddef>
#include <functional>
namespace yafl {
namespace detail {

/**
 * Remove const volatile reference from input argument type T
 * @tparam T
 */
template<typename T>
using remove_cvref_t = typename std::remove_cv_t<std::remove_reference_t<T>>;

/**
 * Forward declaration of function_traits
 * @tparam ...
 */
template <typename ...>
struct function_traits;

template <typename Ret, typename FirstArg, typename... Args>
struct function_traits<Ret, FirstArg, Args...> {
    using Signature = std::function<Ret(FirstArg, Args...)>;
    using PartialApplyFirst = std::function<Ret(Args...)>;
    using ReturnType = Ret;
    using ArgTypes = std::tuple<remove_cvref_t<FirstArg>, remove_cvref_t<Args>...>;
    static constexpr std::size_t ArgCount = sizeof...(Args) + 1;
    template <std::size_t idx>
    using ArgType = std::tuple_element_t<idx, ArgTypes>;
};

template <typename Ret>
struct function_traits<Ret> {
    using Signature = Ret();
    using ReturnType = Ret;
    using ArgTypes = std::tuple<>;
    static constexpr std::size_t ArgCount = 0;
};

} // namespace detail

template <typename Func>
struct function_traits;

template <typename Func>
struct function_traits : function_traits<decltype(&detail::remove_cvref_t<Func>::operator())>{};

template <typename Ret, typename ClassType, typename... Args>
struct function_traits<Ret (ClassType::*)(Args...) const> : detail::function_traits<Ret, Args...>{};

template <typename Ret, typename ClassType, typename... Args>
struct function_traits<Ret (ClassType::*)(Args...)> : detail::function_traits<Ret, Args...>{};

template <typename Ret, typename ClassType, typename... Args>
struct function_traits<Ret (ClassType::*&)(Args...)> : detail::function_traits<Ret, Args...>{};

template <typename Ret, typename... Args>
struct function_traits<Ret (*)(Args...)> : detail::function_traits<Ret, Args...>{};

template <typename Ret, typename... Args>
struct function_traits<Ret (&)(Args...)> : detail::function_traits<Ret, Args...>{};

template <typename Ret, typename... Args>
struct function_traits<Ret (Args...)> : detail::function_traits<Ret, Args...>{};

} // namespace yafl
