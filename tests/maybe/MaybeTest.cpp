/**
 * \file
 * \brief       CAPI Clients Factory
 *
 * \project     BMW Platform Software
 * \copyright   Critical TechWorks SA
 */

#include "yafl/Maybe.h"
#include <string>
#include <tuple>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace yafl;

template <typename T>
Maybe<T> createMaybe() {
    if constexpr (std::is_void_v<T>) {
        return Maybe<T>::Just();
    } else {
        return Maybe<T>::Just({});
    }
}

template<typename T>
class MaybeTest : public ::testing::Test {};

using MaybeParamTypes = ::testing::Types<void, int, std::string>;

TYPED_TEST_SUITE(MaybeTest, MaybeParamTypes);

TYPED_TEST(MaybeTest, assertNothingCreateAValidMaybeForAnyType) {
    Maybe<TypeParam> nothingMaybe = Maybe<TypeParam>::Nothing();
    ASSERT_FALSE(nothingMaybe.hasValue());
}

TYPED_TEST(MaybeTest, assertJustCreateAValidMaybeForAnyType) {
    const auto maybe = createMaybe<TypeParam>();
    ASSERT_TRUE(maybe.hasValue());
}

TYPED_TEST(MaybeTest, assertValueReturnsCorrectType) {
    const auto maybe = createMaybe<TypeParam>();
    if constexpr (!std::is_void_v<TypeParam>) {
        constexpr const auto areSameType = std::is_same_v<TypeParam, decltype(maybe.value())>;
        ASSERT_TRUE(areSameType);
    }
    constexpr const auto areSameType = std::is_same_v<TypeParam, typename MaybeTraits<decltype(maybe)>::ValueType>;
    ASSERT_TRUE(areSameType);
}

TYPED_TEST(MaybeTest, assertValueReturnsExceptionWhenisNothing) {
    const auto maybe = Nothing<TypeParam>();
    if constexpr (!std::is_void_v<TypeParam>) {
        EXPECT_THAT([&maybe]() { std::ignore = maybe.value(); }, testing::Throws<std::runtime_error>());
    }
    constexpr const auto areSameType = std::is_same_v<TypeParam, typename MaybeTraits<decltype(maybe)>::ValueType>;
    ASSERT_TRUE(areSameType);
}

TEST(MaybeTest, assertValueOr) {
    Maybe<int> justMaybe = Just(42);
    ASSERT_EQ(justMaybe.value(), 42);
    ASSERT_EQ(justMaybe.valueOr(32), 42);
    Maybe<int> nothingMaybe = Nothing<int>();
    ASSERT_EQ(nothingMaybe.valueOr(32), 32);
}

TYPED_TEST(MaybeTest, assertFmapComputesCorrectType) {
    if constexpr (std::is_void_v<TypeParam>) {
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultVoid = maybe.fmap([](){});

            ASSERT_TRUE(std::is_void_v<typename MaybeTraits<decltype(resultVoid)>::ValueType>);
        }
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultType = maybe.fmap([](){return 42;});
            constexpr const auto areSameType = std::is_same_v<int, decltype(resultType.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto maybe = Maybe<TypeParam>::Nothing();
            const auto resultInt = maybe.fmap([](){ return;});
            ASSERT_FALSE(resultInt.hasValue());
            const auto resultFloat = maybe.fmap([](){ return 42.24;});
            ASSERT_FALSE(resultFloat.hasValue());
        }
    } else {
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultVoid = maybe.fmap([](const TypeParam&){});
            ASSERT_TRUE(std::is_void_v<typename MaybeTraits<decltype(resultVoid)>::ValueType>);
        }
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultType = maybe.fmap([](const TypeParam&){return 42;});
            constexpr const auto areSameType = std::is_same_v<int, decltype(resultType.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto maybe = Maybe<TypeParam>::Nothing();
            const auto resultInt = maybe.fmap([](const TypeParam&){ return;});
            ASSERT_FALSE(resultInt.hasValue());
            const auto resultFloat = maybe.fmap([](const TypeParam&){ return 42.24;});
            ASSERT_FALSE(resultFloat.hasValue());
        }
    }
}

TYPED_TEST(MaybeTest, assertBindComputesCorrectType) {
    if constexpr (std::is_void_v<TypeParam>) {
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultVoid = maybe.bind([](){ return Maybe<void>::Nothing();});
            ASSERT_TRUE(std::is_void_v<typename MaybeTraits<decltype(resultVoid)>::ValueType>);
        }
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultType = maybe.bind([](){ return Maybe<int>::Just(42);});
            constexpr const auto areSameType = std::is_same_v<int, decltype(resultType.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto maybe = Maybe<TypeParam>::Nothing();
            const auto resultInt = maybe.bind([](){ return Maybe<int>::Nothing();});
            ASSERT_FALSE(resultInt.hasValue());
            const auto resultFloat = maybe.bind([](){ return Maybe<float>::Just(42.24);});
            ASSERT_FALSE(resultFloat.hasValue());
        }
    } else {
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultVoid = maybe.bind([](const TypeParam&){return Maybe<void>::Nothing();});
            ASSERT_TRUE(std::is_void_v<typename MaybeTraits<decltype(resultVoid)>::ValueType>);
        }
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultType = maybe.bind([](const TypeParam&){ return Maybe<int>::Just(42);});
            constexpr const auto areSameType = std::is_same_v<int, decltype(resultType.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto maybe = Maybe<TypeParam>::Nothing();
            const auto resultInt = maybe.bind([](const TypeParam&){ return Maybe<int>::Nothing();});
            ASSERT_FALSE(resultInt.hasValue());
            const auto resultFloat = maybe.bind([](const TypeParam&){ return Maybe<float>::Just(42.24);});
            ASSERT_FALSE(resultFloat.hasValue());
        }
    }
}

TEST(MaybeTest, assertApply) {
    {
        const auto multBy42 = Just([](const int &x) { return x * 42; });
        const auto result = multBy42(Just(2));
        ASSERT_EQ(result.value(), 84);
        const auto result2 = multBy42(2);
        ASSERT_EQ(result2.value(), 84);
        const auto result3 = multBy42(Maybe<int>::Nothing());
        ASSERT_FALSE(result3.hasValue());
    }
    {
        const auto void42 = Just([]() { return 42; });
        const auto result = void42();
        ASSERT_EQ(result.value(), 42);
    }
    {
        const auto voidFunc = []() { return ;};
        const auto nothingVoid = Nothing<decltype(voidFunc)>();
        const auto result = nothingVoid();
        ASSERT_FALSE(result.hasValue());
    }
    {
        const auto intFunc = []() { return 42;};
        const auto nothingInt = Nothing<decltype(intFunc)>();
        const auto result = nothingInt();
        ASSERT_FALSE(result.hasValue());
    }
    {
        const auto justVoidFunc = Just([]() { return ;});
        const auto result = justVoidFunc();
        ASSERT_TRUE(result.hasValue());
    }
    {
        const auto justIntFunc = Just([]() { return 42;});
        const auto result = justIntFunc();
        ASSERT_TRUE(result.hasValue());
    }
    {
        const auto printX = Just([](int x){ std::cout << x << std::endl;});
        const auto result = printX(Just(42));
        ASSERT_TRUE(result.hasValue());
        const auto result2 = printX(Nothing<int>());
        ASSERT_FALSE(result2.hasValue());
    }
    {
        const auto partialApply = Just([](int x, float f, const std::string& s){ return std::to_string(x*f*42) + s;});
        const auto resultPartialFunc = partialApply(Just(2));
        const auto resultPartialFunc2 = resultPartialFunc(Just(0.5));
        const auto resultPartialFunc3 = resultPartialFunc2(Just<std::string>("Text"));

        ASSERT_EQ(resultPartialFunc3.value(), "42.000000Text");

        const auto resultPartialFunc11 = partialApply(2);
        const auto resultPartialFunc21 = resultPartialFunc11(0.5);
        const auto resultPartialFunc31 = resultPartialFunc21("Text");

        ASSERT_EQ(resultPartialFunc31.value(), "42.000000Text");

        const auto resultPartialFunc4 = partialApply(Just(1), Just(2.0))(Just(std::string("Text")));
        ASSERT_EQ(resultPartialFunc4.value(), "84.000000Text");

        const auto resultPartialFunc5 = partialApply(Just(1))(Just(0.5))(Just(std::string("Text")));
        ASSERT_EQ(resultPartialFunc5.value(), "21.000000Text");

        const auto resultPartialFunc6 = partialApply(Just(1), Just(0.5), Just(std::string("Text")));
        ASSERT_EQ(resultPartialFunc6.value(), "21.000000Text");

        const auto resultPartialFunc7 = partialApply.value()(1,3, std::string("Text"));
        ASSERT_EQ(resultPartialFunc7, "126.000000Text");
    }
    {
        const auto partialApply = Just([](int x, float f, const std::string& s){ return std::to_string(x*f*42) + s;});
        const auto resultPartialNothing = partialApply(Nothing<int>());
        const auto resultPartialFunc2 = resultPartialNothing(Just(0.5));
        const auto resultPartialFunc3 = resultPartialFunc2(Just<std::string>("Text"));

        ASSERT_FALSE(resultPartialFunc3.hasValue());
    }
    {
        const auto func = [](int){ return;};
        const auto partialApply = Nothing<decltype(func)>();
        const auto resultPartialJust = partialApply(Nothing<int>());
        ASSERT_FALSE(resultPartialJust.hasValue());

        const auto resultPartialJust2 = partialApply(2);
        ASSERT_FALSE(resultPartialJust2.hasValue());
    }
    {
        const auto func = [](int, float){ return;};
        const auto partialApply = Nothing<decltype(func)>();
        const auto resultPartialJust = partialApply(Nothing<int>());
        ASSERT_FALSE(resultPartialJust.hasValue());

        const auto resultPartialJust2 = partialApply(2);
        ASSERT_FALSE(resultPartialJust2.hasValue());
    }
    {
        const auto partialApply = Just([](int){ return;});
        const auto resultPartialJust = partialApply(2);
        ASSERT_TRUE(resultPartialJust.hasValue());
    }
}