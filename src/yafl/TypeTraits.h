/**
 * \brief       Yet Another Functional Library
 *
 * \copyright   2023, Ernesto Festas.
 *              Distributed under MIT license (See accompanying LICENSE file)
 * \defgroup Function Function type traits
 * \defgroup Tuple Tuple type traits
 * \defgroup Type Yafl type traits
 */
#pragma once

#include <type_traits>
#include <cstddef>
#include <functional>

namespace yafl {

/**
 * @ingroup Function
 */
namespace function {

namespace details {
/**
 * @ingroup Details
 * Forward declaration of Details struct
 * @tparam ...
 */
template<typename ...>
struct CallableDetails;

/**
 * @ingroup Details
 * One or more argument function traits
 * Provides function type information
 * @tparam Ret Return type
 * @tparam FirstArg First argument type
 * @tparam Args Remaining Types
 */
template<typename Ret, typename FirstArg, typename... Args>
struct CallableDetails<Ret, FirstArg, Args...> {
    ///Function Signature
    using Signature = std::function<Ret(FirstArg, Args...)>;
    ///Function Signature lifted to TLift
    template<template<typename...> typename TLift>
    using LiftedSignature = std::function<TLift<Ret>(const TLift<std::decay_t<FirstArg>>&, const TLift<std::decay_t<Args>>&...)>;
    ///Function Signature after applying first argument
    using PartialApplyFirst = std::function<Ret(Args...)>;
    ///Return type
    using ReturnType = Ret;
    ///Function argument types, provided in a tuple
    using ArgTypes = std::tuple<std::decay_t<FirstArg>, std::decay_t<Args>...>;
    ///Number of arguments
    static constexpr std::size_t ArgCount = sizeof...(Args) + 1;
    ///Getter for argument with ID idx
    template <std::size_t idx>
    using ArgType = std::tuple_element_t<idx, ArgTypes>;
};

/**
 * @ingroup Details
 * Zero arguments function traits
 * Provides function type information
 * @tparam Ret Return type
 */
template<typename Ret>
struct CallableDetails<Ret> {
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
struct CallableHelper;

template<typename Ret, typename... Args>
struct CallableHelper<std::function<Ret(&)(Args...)>> : details::CallableDetails<Ret, Args...> {
};

template<typename Ret, typename... Args>
struct CallableHelper<std::function<Ret(Args...)>> : details::CallableDetails<Ret, Args...> {
};

template<typename Ret, typename ClassType, typename... Args>
struct CallableHelper<std::function<Ret (ClassType::*)(Args...) const>> : details::CallableDetails<Ret, Args...> {
};

template<typename Ret, typename ClassType, typename... Args>
struct CallableHelper<std::function<Ret (ClassType::*)(Args...)>> : details::CallableDetails<Ret, Args...> {
};

template<typename Ret, typename ClassType, typename... Args>
struct CallableHelper<std::function<Ret (ClassType::* &)(Args...)>> : details::CallableDetails<Ret, Args...> {
};

template<typename Ret, typename... Args>
struct CallableHelper<std::function<Ret (*)(Args...)>> : details::CallableDetails<Ret, Args...> {
};

/**
 * @ingroup Details
 * Helper struct that enables the construction of a std::function type using
 * the expanded provided tuple as the function input arguments
 * @tparam Ret
 * @tparam Tuple
 */
template <typename Ret, typename Tuple>
struct FunctionFromTupleImpl;

/**
 * @ingroup Details
 * Specialization for tuple with variadic arguments
 * @tparam Ret
 * @tparam Args
 */
template <typename Ret, typename... Args>
struct FunctionFromTupleImpl<Ret, std::tuple<Args...>> {
    using FunctionType = std::function<Ret(Args...)>;
};
} // namespace details

/**
 * @ingroup Function
 * Template class that allows extracting information about a callable type (a function,
 * a lambda, a functor etc.).
 * It provides the callable return type, the number and types of arguments.
 * @tparam Callable Callable type to inspect
 */
template<typename Callable>
struct Info : details::CallableHelper<decltype(std::function{std::declval<std::decay_t<Callable>>()})> {};

/**
 * @ingroup Function
 * Specialization for STL function
 * @tparam Ret function return type
 * @tparam Args function argument types
 */
template<typename Ret, typename ...Args>
struct Info<std::function<Ret(Args...)>> : details::CallableHelper<std::function<Ret(Args...)>> {};

/**
 * @ingroup Function
 * Helper type that allows to construct a std::function type
 * using an expanded tuple as its input arguments
 * @tparam Ret function return type
 * @tparam Tuple Tuple that contains the function arguments
 */
template <typename Ret, typename Tuple>
using FunctionFromTuple = typename details::FunctionFromTupleImpl<Ret, Tuple>::FunctionType;
} // namespace function

/**
 * @ingroup Tuple
 * Yafl Tuple traits
 */
namespace tuple {
/**
 * @ingroup Tuple
 * Function that applies callable to given value and appends the result into given tuple
 * @tparam Callable Function type of the Callable
 * @tparam Tuple Tuple type
 * @tparam Head Type of value o apply the given callable
 * @param callable callable to be executed in the given value
 * @param tuple tuple to store the result of applying callable with value
 * @param value value to be transformed
 * @return a new tuple containing the transformed type
 */
template<typename Callable, typename Tuple, typename Head>
decltype(auto) map_append(Callable&& callable, Tuple&& tuple, Head&& value) {
    return std::tuple_cat(std::forward<Tuple>(tuple), std::make_tuple(callable(std::forward<Head>(value))));
}

/**
 * @ingroup Tuple
 * Function that applies callable to given value and appends the result into given tuple
 * @tparam Callable Function type of the Callable
 * @tparam Tuple Tuple type
 * @tparam Head Type of value o apply the given callable
 * @tparam Tail Remaining arguments types
 * @param callable callable to be executed in the given value
 * @param tuple tuple to store the result of applying callable with value
 * @param value value to be transformed
 * @param ts remaining arguments to be transformed recursively
 * @return a new tuple containing the transformed type
 */
template<typename Callable, typename Tuple, typename Head, typename ...Tail>
decltype(auto) map_append(Callable&& callable, Tuple&& tuple, Head&& value, Tail&&...ts) {
    return map_append(std::forward<Callable>(callable),
                      std::tuple_cat(std::forward<Tuple>(tuple), std::make_tuple(callable(std::forward<Head>(value)))),
                      std::forward<Tail>(ts)...);
}

/**
 * @ingroup Tuple
 * Compares two tuple types by index.
 * @tparam Index index of type to compare
 * @tparam Tuple1 left tuple to compare
 * @tparam Tuple2 right tuple to compare
 * @return true if types are the same or right type is convertible to left type, or false otherwise
 */
template<std::size_t Index, typename Tuple1, typename Tuple2>
constexpr bool compareTupleTypeByIndex(){
    return std::is_convertible_v<std::tuple_element_t<Index, Tuple2>, std::tuple_element_t<Index, Tuple1>> ||
           std::is_same_v<std::tuple_element_t<Index, Tuple1>, std::tuple_element_t<Index, Tuple2>>;
}

namespace details {
/**
 * @ingroup Details
 * Helper class that enables to check whether a tuple is a subset of another tuple.
 * It is a recursive function and this implementation is related with the generic case
 * @tparam Index last used index
 * @tparam Tuple1 Original tuple type
 * @tparam Tuple2 Subset tuple type
 * @tparam IsValid Control recursion
 */
template<std::size_t Index, typename Tuple1, typename Tuple2, bool IsValid>
struct IsTupleSubsetImpl {
    static constexpr std::size_t index = std::tuple_size_v<Tuple2>;
    static constexpr bool value = []() {
        if constexpr (std::tuple_size_v<Tuple2> == 0) {
            return true;
        } else {
            return (Index < std::tuple_size_v<Tuple2>) &&
                   compareTupleTypeByIndex<Index, Tuple1, Tuple2>() &&
                   IsTupleSubsetImpl<Index + 1, Tuple1, Tuple2, (Index + 1 < std::tuple_size_v<Tuple2>)>::value;
        }
    }();
};

/**
 * @ingroup Details
 * Helper class that enables to check whether a tuple is a subset of another tuple.
 * It is a recursive function and this implementation is related with the end condition.
 * @tparam Index last used index
 * @tparam Tuple1 Original tuple type
 * @tparam Tuple2 Subset tuple type
 */
template<std::size_t Index, typename Tuple1, typename Tuple2>
struct IsTupleSubsetImpl<Index, Tuple1, Tuple2, false> {
    static constexpr bool value = true;
    static constexpr std::size_t index = std::tuple_size_v<Tuple2>;
};
} // namespace details

/**
 * @ingroup Tuple
 * Extract information about whether a tuple is a subset of another tuple
 * If true extracts the index
 * @tparam Tuple1 Original tuple type
 * @tparam Tuple2 Subset tuple type
 */
template <typename Tuple1, typename Tuple2>
struct IsTupleSubset : details::IsTupleSubsetImpl<0, Tuple1, Tuple2, true>{};

namespace details {

/**
 * @ingroup Details
 * Helper class that enables the creation of a tuple type that is a subset of a given tuple
 * Subset is created starting from the given index.
 * This templated class is recursive and this implementation is related with the recursion body. It defines
 * a new type NewSubsetTuple that contains the tuple subset type.
 * @tparam Index last used index
 * @tparam Tuple original tuple type
 * @tparam SubsetTuple updated type
 * @tparam IsValid controls execution of the recursive function
 */
template <std::size_t Index, typename Tuple, typename SubsetTuple, bool IsValid>
struct CreateSubsetTupleFromIndex {
    using CurrentElementType = std::tuple_element_t<Index, Tuple>;

    using NewSubsetTuple = typename CreateSubsetTupleFromIndex<
            Index + 1,
            Tuple,
            decltype(std::tuple_cat(std::declval<SubsetTuple>(), std::make_tuple(CurrentElementType()))),
            (Index + 1 < std::tuple_size_v<Tuple>)>::NewSubsetTuple;
};

/**
 * @ingroup Details
 * Helper class that enables the creation of a tuple type that is a subset of a given tuple
 * Subset is created starting from the given index.
 * This templated class is recursive and this implementation is related with the recursion end condition.
 * It is a specialization using IsValid type as false. It defines a new type NewSubsetTuple that contains
 * the tuple subset type.
 * @tparam Index last used index
 * @tparam Tuple original tuple type
 * @tparam SubsetTuple created type
 */
template <std::size_t Index, typename Tuple, typename SubsetTuple>
struct CreateSubsetTupleFromIndex<Index, Tuple, SubsetTuple, false> {
    using NewSubsetTuple = SubsetTuple;
};
} // namespace details

/**
 * @ingroup Tuple
 * Creates a tuple subset type starting for the given index position
 * @tparam Index index position
 * @tparam Tuple tuple to extract subset
 */
template <std::size_t Index, typename Tuple>
using TupleSubset = typename details::CreateSubsetTupleFromIndex<Index, Tuple, std::tuple<>, true>::NewSubsetTuple;
}

/**
 * @ingroup Type
 * Yafl Type traits
 */
namespace type {

/**
 * @ingroup Type
 * Helper struct that enables debugging by showing the information about the type T
 * @tparam T type to show
 * Note: This will make the compile to fail but will show information for the types
 */
template<typename ...T>
struct WhatIsThis;

/**
 * @ingroup Type
 * Helper struct that enables debugging by showing the information about the value of the Index
 * @tparam Index value to show
 * Note: This will make the compile to fail but will show information for the values
 */
template<std::size_t Index>
struct WhatIsThisValue;

namespace details {
/**
 * @ingroup Details
 * Default class that contains information about whether a given type is a Functor, Applicative or Monad.
 * It is specialized for both Maybe and Either classes.
 */
template<typename>
struct DomainDetailsImpl {
    ///boolean flag that states whether type T is a Functor or not
    static constexpr bool hasFunctorBase = false;
    ///boolean flag that states whether type T is a Applicative or not
    static constexpr bool hasApplicativeBase = false;
    ///boolean flag that states whether type T is a Monad or not
    static constexpr bool hasMonadicBase = false;
};
} // namespace details

/**
 * @ingroup Type
 * Type details for Yafl types.
 * Extracts information about whether type T is a Functor, Applicative or Monad
 * @tparam T
 */
template<typename T>
struct DomainTypeInfo : public details::DomainDetailsImpl<std::decay_t<T>>{};

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
    static constexpr bool value = decltype(test<std::decay_t<T>>(0))::value;
};

/**
 * @ingroup Type
 * Helper class that introspects whether a type T is a Callable with the provided Args or not
 * @tparam T Type to check
 * @tparam Args function argument types
 */
template <typename Callable, typename... Args>
struct IsCallableWithArgs {
private:
    template <typename C, typename... A>
    static auto test(int) -> decltype(std::declval<C>()(std::declval<A>()...), std::true_type());

    template <typename C, typename... A>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<std::decay_t<Callable>, Args...>(0))::value;
};

} // namespace type
} // namespace yafl
