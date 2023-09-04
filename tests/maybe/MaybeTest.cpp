/**
 * \brief       Yet Another Functional Library
 *
 * \copyright   2023, Ernesto Festas.
 *              Distributed under MIT license (See accompanying LICENSE file)
 */

#include "yafl/HOF.h"
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

TYPED_TEST_SUITE(MaybeTest, MaybeParamTypes, );

TYPED_TEST(MaybeTest, assertNothingCreateAValidMaybeForAnyType) {
    Maybe<TypeParam> nothingMaybe = Maybe<TypeParam>::Nothing();
    ASSERT_FALSE(nothingMaybe.hasValue());
}

TYPED_TEST(MaybeTest, assertJustCreateAValidMaybeForAnyType) {
    const auto maybe = createMaybe<TypeParam>();
    ASSERT_TRUE(maybe.hasValue());
}

TYPED_TEST(MaybeTest, assertCopyConstructor) {
    const auto maybe = createMaybe<TypeParam>();
    const Maybe destination(maybe);
    ASSERT_EQ(maybe, destination);
}

TYPED_TEST(MaybeTest, assertAssignmentOperator) {
    const auto maybe = createMaybe<TypeParam>();
    auto destination = createMaybe<TypeParam>();
    destination = maybe;
    ASSERT_EQ(maybe, destination);
}

TYPED_TEST(MaybeTest, assertMoveConstructor) {
    auto maybe = createMaybe<TypeParam>();
    const auto destination(std::move(maybe));
    EXPECT_TRUE(destination.hasValue());
}

TYPED_TEST(MaybeTest, MoveAssignment) {
    auto maybe = createMaybe<TypeParam>();
    auto destination = createMaybe<TypeParam>();
    destination = std::move(maybe);
    EXPECT_TRUE(destination.hasValue());
}

TYPED_TEST(MaybeTest, assertValueReturnsCorrectType) {
    const auto maybe = createMaybe<TypeParam>();
    if constexpr (!std::is_void_v<TypeParam>) {
        constexpr const auto areSameType = std::is_same_v<TypeParam, decltype(maybe.value())>;
        ASSERT_TRUE(areSameType);
    }
    constexpr const auto areSameType = std::is_same_v<TypeParam, typename type::DomainTypeInfo<decltype(maybe)>::ValueType>;
    ASSERT_TRUE(areSameType);
}

TYPED_TEST(MaybeTest, assertValueReturnsExceptionWhenisNothing) {
    const auto maybe = maybe::Nothing<TypeParam>();
    if constexpr (!std::is_void_v<TypeParam>) {
        EXPECT_THAT([&maybe]() { std::ignore = maybe.value(); }, testing::Throws<std::runtime_error>());
    }
    constexpr const auto areSameType = std::is_same_v<TypeParam, typename type::DomainTypeInfo<decltype(maybe)>::ValueType>;
    ASSERT_TRUE(areSameType);
}

TEST(MaybeTest, assertValueOr) {
    Maybe<int> justMaybe = maybe::Just(42);
    ASSERT_EQ(justMaybe.value(), 42);
    ASSERT_EQ(justMaybe.valueOr(32), 42);
    Maybe<int> nothingMaybe = maybe::Nothing<int>();
    ASSERT_EQ(nothingMaybe.valueOr(32), 32);
}

TYPED_TEST(MaybeTest, assertFmapComputesCorrectType) {
    if constexpr (std::is_void_v<TypeParam>) {
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultVoid = maybe.fmap([](){});
            ASSERT_TRUE(std::is_void_v<typename type::DomainTypeInfo<decltype(resultVoid)>::ValueType>);
        }
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultVoid = functor::fmap([](){}, maybe);
            ASSERT_TRUE(std::is_void_v<typename type::DomainTypeInfo<decltype(resultVoid)>::ValueType>);
        }
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto f = functor::fmap<yafl::Maybe>([](){});
            const auto resultVoid = f(maybe);
            ASSERT_TRUE(std::is_void_v<typename type::DomainTypeInfo<decltype(resultVoid)>::ValueType>);
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
            ASSERT_TRUE(std::is_void_v<typename type::DomainTypeInfo<decltype(resultVoid)>::ValueType>);
        }
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultVoid = functor::fmap([](const TypeParam&){}, maybe);
            ASSERT_TRUE(std::is_void_v<typename type::DomainTypeInfo<decltype(resultVoid)>::ValueType>);
        }
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto f = functor::fmap<yafl::Maybe>([](const TypeParam&){});
            const auto resultVoid = f(maybe);
            ASSERT_TRUE(std::is_void_v<typename type::DomainTypeInfo<decltype(resultVoid)>::ValueType>);
        }
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultVoid = maybe.fmap([](TypeParam&&){});
            ASSERT_TRUE(std::is_void_v<typename type::DomainTypeInfo<decltype(resultVoid)>::ValueType>);
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
            ASSERT_TRUE(std::is_void_v<typename type::DomainTypeInfo<decltype(resultVoid)>::ValueType>);
        }
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultVoid = monad::bind([](){ return Maybe<void>::Nothing();}, maybe);
            ASSERT_TRUE(std::is_void_v<typename type::DomainTypeInfo<decltype(resultVoid)>::ValueType>);
        }
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto f = monad::bind<yafl::Maybe>([](){ return Maybe<void>::Nothing();});
            const auto resultVoid = f(maybe);
            ASSERT_TRUE(std::is_void_v<typename type::DomainTypeInfo<decltype(resultVoid)>::ValueType>);
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
            const auto resultFloat = maybe.bind([](){ return Maybe<float>::Just(42.24f);});
            ASSERT_FALSE(resultFloat.hasValue());
        }
    } else {
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultVoid = maybe.bind([](const TypeParam&){return Maybe<void>::Nothing();});
            ASSERT_TRUE(std::is_void_v<typename type::DomainTypeInfo<decltype(resultVoid)>::ValueType>);
        }
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto resultVoid = monad::bind([](const TypeParam&){return Maybe<void>::Nothing();}, maybe);
            ASSERT_TRUE(std::is_void_v<typename type::DomainTypeInfo<decltype(resultVoid)>::ValueType>);
        }
        {
            const auto maybe = createMaybe<TypeParam>();
            const auto f = monad::bind<yafl::Maybe>([](const TypeParam&){return Maybe<void>::Nothing();});
            const auto resultVoid = f(maybe);
            ASSERT_TRUE(std::is_void_v<typename type::DomainTypeInfo<decltype(resultVoid)>::ValueType>);
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
            const auto resultFloat = maybe.bind([](const TypeParam&){ return Maybe<float>::Just(42.24f);});
            ASSERT_FALSE(resultFloat.hasValue());
        }
    }
}

TEST(MaybeTest, assertApply) {
    {
        const auto multBy42 = maybe::Just([](const int &x) { return x * 42; });
        const auto result = multBy42(maybe::Just(2));
        ASSERT_EQ(result.value(), 84);
        ASSERT_EQ(applicative::apply(multBy42, maybe::Just(2)).value(), 84);
        ASSERT_EQ(applicative::apply(multBy42)(maybe::Just(2)).value(), 84);
        const auto result2 = multBy42(2);
        ASSERT_EQ(result2.value(), 84);
        ASSERT_EQ(applicative::apply(multBy42, 2).value(), 84);
        ASSERT_EQ(applicative::apply(multBy42)(2).value(), 84);
        const auto result3 = multBy42(Maybe<int>::Nothing());
        ASSERT_FALSE(result3.hasValue());
        ASSERT_FALSE(applicative::apply(multBy42, Maybe<int>::Nothing()).hasValue());
        ASSERT_FALSE(applicative::apply(multBy42)(Maybe<int>::Nothing()).hasValue());
    }
    {
        const auto void42 = maybe::Just([]() { return 42; });
        const auto result = void42();
        ASSERT_EQ(result.value(), 42);
        ASSERT_EQ(applicative::apply(void42)().value(), 42);
    }
    {
        const auto voidFunc = []() { return ;};
        const auto nothingVoid = maybe::Nothing<decltype(voidFunc)>();
        const auto result = nothingVoid();
        ASSERT_FALSE(result.hasValue());
    }
    {
        const auto intFunc = []() { return 42;};
        const auto nothingInt = maybe::Nothing<decltype(intFunc)>();
        const auto result = nothingInt();
        ASSERT_FALSE(result.hasValue());
    }
    {
        const auto justVoidFunc = maybe::Just([]() { return ;});
        const auto result = justVoidFunc();
        ASSERT_TRUE(result.hasValue());
    }
    {
        const auto justIntFunc = maybe::Just([]() { return 42;});
        const auto result = justIntFunc();
        ASSERT_TRUE(result.hasValue());
    }
    {
        const auto printX = maybe::Just([](int x){ std::ignore = x;});
        const auto result = printX(maybe::Just(42));
        ASSERT_TRUE(result.hasValue());
        const auto result2 = printX(maybe::Nothing<int>());
        ASSERT_FALSE(result2.hasValue());
    }
    {
        const auto partialApply = maybe::Just([](int x, float f, const std::string& s){ return std::to_string(static_cast<float>(x) * f * 42) + s;});
        const auto resultPartialFunc = partialApply(maybe::Just(2));
        const auto resultPartialFunc2 = resultPartialFunc(maybe::Just<float>(0.5));
        const auto resultPartialFunc3 = resultPartialFunc2(maybe::Just<std::string>("Text"));

        ASSERT_EQ(resultPartialFunc3.value(), "42.000000Text");

        const auto resultPartialFunc11 = partialApply(2);
        const auto resultPartialFunc21 = resultPartialFunc11(0.5f);
        const auto resultPartialFunc31 = resultPartialFunc21("Text");

        ASSERT_EQ(resultPartialFunc31.value(), "42.000000Text");

        const auto resultPartialFunc4 = partialApply(maybe::Just(1), maybe::Just(2.0f))(maybe::Just(std::string("Text")));
        ASSERT_EQ(resultPartialFunc4.value(), "84.000000Text");

        const auto resultPartialFunc5 = partialApply(maybe::Just(1))(maybe::Just(0.5f))(maybe::Just(std::string("Text")));
        ASSERT_EQ(resultPartialFunc5.value(), "21.000000Text");

        const auto resultPartialFunc6 = partialApply(maybe::Just(1), maybe::Just(0.5f), maybe::Just(std::string("Text")));
        ASSERT_EQ(resultPartialFunc6.value(), "21.000000Text");

        const auto resultPartialFunc7 = partialApply.value()(1,3, std::string("Text"));
        ASSERT_EQ(resultPartialFunc7, "126.000000Text");
    }
    {
        const auto partialApply = maybe::Just([](int x, float f, const std::string& s){ return std::to_string(static_cast<float>(x) * f * 42) + s;});
        const auto resultPartialNothing = partialApply(maybe::Nothing<int>());
        const auto resultPartialFunc2 = resultPartialNothing(maybe::Just(0.5f));
        const auto resultPartialFunc3 = resultPartialFunc2(maybe::Just<std::string>("Text"));

        ASSERT_FALSE(resultPartialFunc3.hasValue());
    }
    {
        const auto func = [](int){ return;};
        const auto partialApply = maybe::Nothing<decltype(func)>();
        const auto resultPartialJust = partialApply(maybe::Nothing<int>());
        ASSERT_FALSE(resultPartialJust.hasValue());

        const auto resultPartialJust2 = partialApply(2);
        ASSERT_FALSE(resultPartialJust2.hasValue());
    }
    {
        const auto func = [](int, float){ return;};
        const auto partialApply = maybe::Nothing<decltype(func)>();
        const auto resultPartialJust = partialApply(maybe::Nothing<int>());
        ASSERT_FALSE(resultPartialJust.hasValue());

        const auto resultPartialJust2 = partialApply(2);
        ASSERT_FALSE(resultPartialJust2.hasValue());
    }
    {
        const auto partialApply = maybe::Just([](int){ return;});
        const auto resultPartialJust = partialApply(2);
        ASSERT_TRUE(resultPartialJust.hasValue());
    }
    {
        const auto v = maybe::Just(42);
        const auto ap = maybe::Just(&id<int>);
        ASSERT_TRUE(ap(v).hasValue());
        ASSERT_EQ(ap(v).value(), v.value());

        ASSERT_TRUE(ap(Maybe(v)).hasValue());
        ASSERT_EQ(ap(Maybe(v)).value(), v.value());

        ASSERT_TRUE(ap(maybe::Just(42)).hasValue());
        ASSERT_EQ(ap(maybe::Just(42)).value(), v.value());

        ASSERT_TRUE(ap(42).hasValue());
        ASSERT_EQ(ap(42).value(), v.value());
    }
    {
        const auto lf = [](int i){ return 42 * i;};
        const auto rf = [](int i){ return std::to_string(i);};
        const auto ap = maybe::Just(&compose<decltype(lf), decltype(rf)>);
        const auto result = ap(lf)(rf)(2);

        const auto ap2 = maybe::Just(std::function(lf))(2);
        const auto result2 = maybe::Just([](int i){ return std::to_string(i);})(ap2);

        ASSERT_TRUE(result.hasValue());
        ASSERT_TRUE(result2.hasValue());

        ASSERT_EQ(result.value(), result2.value());
    }
    {
        const auto ap = maybe::Just([](){return 42;});
        ASSERT_TRUE(ap().hasValue());

        const auto ap2 = maybe::Nothing<std::function<int()>>();
        ASSERT_FALSE(ap2().hasValue());
    }
    {
        const auto ap = maybe::Just([](){return [](int i){return 42 * i;};});
        ASSERT_TRUE(ap().hasValue());

        const auto func = [](){return [](int i){return 42 * i;};};
        const auto apn = maybe::Nothing<decltype(func)>();
        ASSERT_FALSE(apn().hasValue());
    }
    {
        const auto ap = maybe::Just([](){return [](int){return;};});
        ASSERT_TRUE(ap().hasValue());
    }
    {
        const auto f1 = [](int i){ return static_cast<float>(i) * 4.2f;};
        const auto f2 = [](float i){ return i;};
        const auto ap = maybe::Just(&compose<decltype(f1), decltype(f2)>);
        ASSERT_TRUE(ap(f1)(f2)(2).hasValue());
        ASSERT_TRUE(ap(maybe::Just(f1))(maybe::Just(f2))(maybe::Just(2)).hasValue());
    }
    {
        const auto f1 = [](int i){ return static_cast<float>(i) * 4.2f;};
        const auto f2 = [](float i){ return i;};
        const auto ap = maybe::Just(&compose<decltype(f1), decltype(f2)>);
        ASSERT_FALSE(ap(maybe::Nothing<decltype(f1)>())(maybe::Just(f2))(maybe::Just(2)).hasValue());
        ASSERT_FALSE(ap(maybe::Just(f1))(maybe::Nothing<decltype(f2)>())(maybe::Just(2)).hasValue());
        ASSERT_FALSE(ap(maybe::Just(f1))(maybe::Just(f2))(maybe::Nothing<int>()).hasValue());
    }
    {
        const auto f1 = [](int i){ return Maybe<float>::Just(static_cast<float>(i) * 4.2f);};
        const auto f2 = [](float i){ return Maybe<float>::Just(i);};
        const auto ap = maybe::Nothing<decltype(&compose<decltype(f1), decltype(f2)>)>();
        ASSERT_FALSE(ap(f1)(f2)(2).hasValue());
    }
    {
        const auto f1 = [](int i){ return Maybe<float>::Just(static_cast<float>(i) * 4.2f);};
        const auto f2 = [](float i){ return Maybe<float>::Just(i);};
        const auto ap = maybe::Just(&compose<decltype(f1), decltype(f2)>);
        ASSERT_TRUE(ap(maybe::Just(f1))(maybe::Just(f2))(maybe::Just(2)).hasValue());
    }
    {
        const auto f1 = [](int i){ return Maybe<float>::Just(static_cast<float>(i) * 4.2f);};
        const auto f2 = [](float i){ return Maybe<float>::Just(i);};
        const auto ap = maybe::Just(&compose<decltype(f1), decltype(f2)>);
        ASSERT_FALSE(ap(maybe::Nothing<decltype(f1)>())(maybe::Just(f2))(maybe::Just(2)).hasValue());
        ASSERT_FALSE(ap(maybe::Just(f1))(maybe::Nothing<decltype(f2)>())(maybe::Just(2)).hasValue());
        ASSERT_FALSE(ap(maybe::Just(f1))(maybe::Just(f2))(maybe::Nothing<int>()).hasValue());
    }
    {
        const auto f1 = [](int i){ return Maybe<float>::Just(static_cast<float>(i) * 4.2f);};
        const auto f2 = [](float i){ return Maybe<float>::Just(i);};
        const auto ap = maybe::Nothing<decltype(&compose<decltype(f1), decltype(f2)>)>();
        ASSERT_FALSE(ap(f1)(f2)(2).hasValue());
    }
}

TEST(MaybeTest, validate_kleisli_compose){
    {
        const auto f1 = [](int i) { return maybe::Just(i * 2);};
        const auto f2 = [](int i) { return maybe::Just(std::to_string(i * 2));};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(2).value(), "8");
    }
    {
        const auto f1 = [](const Maybe<int>& i) { return maybe::Just(i.value() * 2);};
        const auto f2 = [](int i) { return maybe::Just(std::to_string(i * 2));};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(maybe::Just(2)).value(), "8");
    }
    {
        const auto f1 = [](int i) { return maybe::Just(i * 2);};
        const auto f2 = [](const Maybe<int>& i) { return maybe::Just(std::to_string(i.value() * 2));};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(2).value(), "8");
    }
    {
        const auto f1 = [](const Maybe<int>& i) { return maybe::Just(i.value() * 2);};
        const auto f2 = [](const Maybe<int>& i) { return maybe::Just(std::to_string(i.value() * 2));};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(maybe::Just(2)).value(), "8");
    }
    {
        const auto f1 = [](int) { return maybe::Nothing<int>();};
        const auto f2 = [](int i) { return maybe::Just(i * 2);};
        const auto f = compose(f1, f2);
        ASSERT_FALSE(f(2).hasValue());
    }
    {
        const auto f1 = [](int i) { return maybe::Just(i * 2);};
        const auto f2 = [](int) { return maybe::Nothing<int>();};
        const auto f = compose(f1, f2);
        ASSERT_FALSE(f(2).hasValue());
    }
    {
        const auto f1 = [](int) { return maybe::Nothing<void>();};
        const auto f2 = []() { return maybe::Just<void>();};
        const auto f = compose(f1, f2);
        ASSERT_FALSE(f(2).hasValue());
    }
    {
        const auto f1 = [](int) { return maybe::Just<void>();};
        const auto f2 = []() { return maybe::Just<void>();};
        const auto f = compose(f1, f2);
        ASSERT_TRUE(f(2).hasValue());
    }
}

TEST(MaybeTest, validate_lift) {
    {
        const auto funcNoArgRetVoid = []() { return;};
        const auto lifted = maybe::lift(funcNoArgRetVoid);
        const auto result = lifted();
        ASSERT_TRUE(result.hasValue());
    }
    {
        const auto funcNoArgRetInt = []() { return 42;};
        const auto lifted = maybe::lift(funcNoArgRetInt);
        const auto result = lifted();
        ASSERT_TRUE(result.hasValue());
        ASSERT_EQ(result.value(), 42);
    }
    {
        const auto funcOneArgRetVoid = [](int) { return;};
        const auto lifted = maybe::lift(funcOneArgRetVoid);
        const auto result = lifted(maybe::Just(2));
        ASSERT_TRUE(result.hasValue());
        const auto result2 = lifted(maybe::Nothing<int>());
        ASSERT_FALSE(result2.hasValue());
    }
    {
        const auto funcOneArgRetInt = [](int i) { return 42 * i;};
        const auto lifted = maybe::lift(funcOneArgRetInt);
        const auto result = lifted(maybe::Just(1));
        ASSERT_TRUE(result.hasValue());
        ASSERT_EQ(result.value(), 42);
        const auto result2 = lifted(maybe::Nothing<int>());
        ASSERT_FALSE(result2.hasValue());
    }
    {
        const auto funcMultiArgRetVoid = [](int, float, const std::string&) { return;};
        const auto lifted = maybe::lift(funcMultiArgRetVoid);
        const auto result = lifted(maybe::Just(2), maybe::Just<float>(4.2f), maybe::Just<std::string>("dummy"));
        ASSERT_TRUE(result.hasValue());
        const auto result2 = lifted(maybe::Nothing<int>(), maybe::Just<float>(4.2f), maybe::Just<std::string>("dummy"));
        ASSERT_FALSE(result2.hasValue());
        const auto result3 = lifted(maybe::Just(1), maybe::Nothing<float>(), maybe::Just<std::string>("dummy"));
        ASSERT_FALSE(result3.hasValue());
        const auto result4 = lifted(maybe::Just(1), maybe::Just<float>(4.2f), maybe::Nothing<std::string>());
        ASSERT_FALSE(result4.hasValue());
    }
    {
        const auto funcMultiArgRetString = [](int i, int j, const std::string& s) { return s + std::to_string(i + j);};
        const auto lifted = maybe::lift(funcMultiArgRetString);
        const auto result = lifted(maybe::Just(2), maybe::Just(4), maybe::Just<std::string>("dummy"));
        ASSERT_TRUE(result.hasValue());
        ASSERT_EQ(result.value(), "dummy6");
        const auto result2 = lifted(maybe::Nothing<int>(), maybe::Just<int>(4), maybe::Just<std::string>("dummy"));
        ASSERT_FALSE(result2.hasValue());
        const auto result3 = lifted(maybe::Just(1), maybe::Nothing<int>(), maybe::Just<std::string>("dummy"));
        ASSERT_FALSE(result3.hasValue());
        const auto result4 = lifted(maybe::Just(1), maybe::Just<int>(4), maybe::Nothing<std::string>());
        ASSERT_FALSE(result4.hasValue());
    }
}
