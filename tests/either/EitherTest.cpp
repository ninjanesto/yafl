#include "yafl/Either.h"
#include <gtest/gtest.h>


template<typename T>
class EitherTest : public ::testing::Test {};

struct VoidVoid {
    using Error = void;
    using Ok = void;
};

struct IntVoid {
    using Error = int;
    using Ok = void;
};

struct IntInt {
    using Error = int;
    using Ok = int;
};

struct VoidInt {
    using Error = void;
    using Ok = int;
};

using namespace yafl;

template <typename E, typename V>
Either<E, V> createError() {
    if constexpr (std::is_void_v<E>) {
        return Either<E, V>::Error();
    } else {
        return Either<E, V>::Error({});
    }
}

template <typename E, typename V>
Either<E, V> createOk() {
    if constexpr (std::is_void_v<V>) {
        return Either<E, V>::Ok();
    } else {
        return Either<E, V>::Ok({});
    }
}

using EitherTypes = ::testing::Types<VoidVoid, IntVoid, IntInt, VoidInt>;

TYPED_TEST_SUITE(EitherTest, EitherTypes);

TYPED_TEST(EitherTest, assertErrorCreateAValidEitherForAnyType) {
    Either<typename TypeParam::Error, typename TypeParam::Ok> either = createError<typename TypeParam::Error, typename TypeParam::Ok>();
    ASSERT_TRUE(either.isError());
}

TYPED_TEST(EitherTest, assertOkCreateAValidEitherForAnyType) {
    Either<typename TypeParam::Error, typename TypeParam::Ok> either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
    ASSERT_TRUE(either.isOk());
}

TYPED_TEST(EitherTest, assertFmapComputesCorrectType) {
    if constexpr (std::is_void_v<typename TypeParam::Ok>) {
        {
            const auto either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultVoid = either.fmap([](){});
            ASSERT_TRUE(resultVoid.isOk());
        }
        {
            const auto either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultType = either.fmap([](){return std::to_string(42);});
            ASSERT_TRUE(resultType.isOk());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultType.value())>;
            ASSERT_TRUE(areSameType);

        }
    } else {
        {
            const auto either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultVoid = either.fmap([](const typename TypeParam::Ok&){});
            ASSERT_TRUE(resultVoid.isOk());
        }
        {
            const auto either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultType = either.fmap([](const typename TypeParam::Ok&){return std::to_string(42);});
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultType.value())>;
            ASSERT_TRUE(areSameType);
        }
    }

    if constexpr (std::is_void_v<typename TypeParam::Error>) {
        if constexpr (std::is_void_v<typename TypeParam::Ok>) {
            const auto either = createError<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultVoid = either.fmap([](){});
            ASSERT_TRUE(resultVoid.isError());
        } else {
            const auto either = createError<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultType = either.fmap([](const typename TypeParam::Ok&){return std::to_string(42);});
            ASSERT_TRUE(resultType.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultType.value())>;
            ASSERT_TRUE(areSameType);
        }
    } else {
        if constexpr (std::is_void_v<typename TypeParam::Ok>) {
            const auto either = createError<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultVoid = either.fmap([](){});
            ASSERT_TRUE(resultVoid.isError());
        } else {
            const auto either = createError<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultType = either.fmap([](const typename TypeParam::Ok&){return std::to_string(42);});
            ASSERT_TRUE(resultType.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultType.value())>;
            ASSERT_TRUE(areSameType);
        }
    }
}

TYPED_TEST(EitherTest, assertBindComputesCorrectType) {
    if constexpr (std::is_void_v<typename TypeParam::Ok> && std::is_void_v<typename TypeParam::Error>) {
        {
            const auto either = createOk<void, void>();
            const auto resultOk = either.bind([]() { return Ok<void, void>(); });
            ASSERT_TRUE(resultOk.isOk());
        }
        {
            const auto either = createOk<void, void>();
            const auto resultError = either.bind([]() { return Error<void, void>(); });
            ASSERT_TRUE(resultError.isError());
        }
        {
            const auto either = createError<void, void>();
            const auto resultError = either.bind([]() { return Ok<void, void>(); });
            ASSERT_TRUE(resultError.isError());
        }

        {
            const auto either = createError<void, void>();
            const auto resultError = either.bind([]() { return Error<void, void>(); });
            ASSERT_TRUE(resultError.isError());
        }
    }

    if constexpr (std::is_void_v<typename TypeParam::Ok> && !std::is_void_v<typename TypeParam::Error>) {
        {
            const auto either = createOk<typename TypeParam::Error, void>();
            const auto resultOk = either.bind([]() { return Ok<typename TypeParam::Error, void>(); });
            ASSERT_TRUE(resultOk.isOk());
        }
        {
            const auto either = createOk<typename TypeParam::Error, void>();
            const auto resultError = either.bind([]() { return Error<typename TypeParam::Error, void>(42); });
            ASSERT_TRUE(resultError.isError());
        }
        {
            const auto either = createError<typename TypeParam::Error, void>();
            const auto resultError = either.bind([]() { return Ok<typename TypeParam::Error, void>(); });
            ASSERT_TRUE(resultError.isError());
        }
        {
            const auto either = createError<typename TypeParam::Error, void>();
            const auto resultError = either.bind([]() { return Error<typename TypeParam::Error, void>(42); });
            ASSERT_TRUE(resultError.isError());
        }
    }

    if constexpr (!std::is_void_v<typename TypeParam::Ok> && std::is_void_v<typename TypeParam::Error>) {
        {
            const auto either = createOk<void, typename TypeParam::Ok>();
            const auto resultOk = either.bind([](const typename TypeParam::Ok &) { return Ok<void, std::string>("42"); });
            ASSERT_TRUE(resultOk.isOk());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultOk.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createOk<void, typename TypeParam::Ok>();
            const auto resultError = either.bind([](const typename TypeParam::Ok &) { return Error<void, std::string>(); });
            ASSERT_TRUE(resultError.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultError.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createError<void, typename TypeParam::Ok>();
            const auto resultError = either.bind([](const typename TypeParam::Ok &) { return Ok<void, std::string>("42"); });
            ASSERT_TRUE(resultError.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultError.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createError<void, typename TypeParam::Ok>();
            const auto resultError = either.bind([](const typename TypeParam::Ok &) { return Error<void, std::string>(); });
            ASSERT_TRUE(resultError.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultError.value())>;
            ASSERT_TRUE(areSameType);
        }
    }

    if constexpr (!std::is_void_v<typename TypeParam::Ok> && !std::is_void_v<typename TypeParam::Error>) {
        {
            const auto either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultOk = either.bind([](const typename TypeParam::Ok&){ return Ok<typename TypeParam::Error, std::string>(std::string("42")); });
            ASSERT_TRUE(resultOk.isOk());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultOk.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultError = either.bind([](const typename TypeParam::Ok &) { return Error<typename TypeParam::Error, std::string>(29); });
            ASSERT_TRUE(resultError.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultError.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createError<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultError = either.bind([](const typename TypeParam::Ok &) { return Ok<typename TypeParam::Error, std::string>("29"); });
            ASSERT_TRUE(resultError.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultError.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createError<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultError = either.bind([](const typename TypeParam::Ok &) { return Error<typename TypeParam::Error, std::string>(29);
            });
            ASSERT_TRUE(resultError.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultError.value())>;
            ASSERT_TRUE(areSameType);
        }
    }
}

TYPED_TEST(EitherTest, validateValueThrowsExceptionWhenEitherIsError) {
    if constexpr (!std::is_void_v<typename TypeParam::Ok>) {
        const auto either = createError<typename TypeParam::Error, typename TypeParam::Ok>();
        EXPECT_THROW({either.value();}, std::runtime_error );
    }
}

TYPED_TEST(EitherTest, validateErrorThrowsExceptionWhenEitherIsValue) {
    if constexpr (!std::is_void_v<typename TypeParam::Error>) {
        const auto either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
        EXPECT_THROW({either.error();}, std::runtime_error );
    }
}

TEST(EitherTest, assertApply) {
    {
        const auto multBy42 = Ok<void>([](const int &x) { return x * 42; });
        const auto result = multBy42(Ok<void>(2));
        ASSERT_EQ(result.value(), 84);
        const auto result2 = multBy42(Error<void, int>());
        ASSERT_TRUE(result2.isError());
    }
//    {
//        const auto void42 = Just([]() { return 42; });
//        const auto result = void42();
//        ASSERT_EQ(result.value(), 42);
//    }
//    {
//        const auto voidFunc = []() { return ;};
//        const auto nothingVoid = Nothing<decltype(voidFunc)>();
//        const auto result = nothingVoid();
//        ASSERT_FALSE(result.hasValue());
//    }
//    {
//        const auto intFunc = []() { return 42;};
//        const auto nothingInt = Nothing<decltype(intFunc)>();
//        const auto result = nothingInt();
//        ASSERT_FALSE(result.hasValue());
//    }
//    {
//        const auto justVoidFunc = Just([]() { return ;});
//        const auto result = justVoidFunc();
//        ASSERT_TRUE(result.hasValue());
//    }
//    {
//        const auto justIntFunc = Just([]() { return 42;});
//        const auto result = justIntFunc();
//        ASSERT_TRUE(result.hasValue());
//    }
//    {
//        const auto printX = Just([](int x){ std::cout << x << std::endl;});
//        const auto result = printX(Just(42));
//        ASSERT_TRUE(result.hasValue());
//        const auto result2 = printX(Nothing<int>());
//        ASSERT_FALSE(result2.hasValue());
//    }
//    {
//        const auto partialApply = Just([](int x, float f, const std::string& s){ return std::to_string(x*f*42) + s;});
//        const auto resultPartialFunc = partialApply(Just(2));
//        const auto resultPartialFunc2 = resultPartialFunc(Just(0.5));
//        const auto resultPartialFunc3 = resultPartialFunc2(Just<std::string>("Text"));
//
//        ASSERT_EQ(resultPartialFunc3.value(), "42.000000Text");
//
//        const auto resultPartialFunc4 = partialApply(Just(1), Just(2.0))(Just(std::string("Text")));
//        ASSERT_EQ(resultPartialFunc4.value(), "84.000000Text");
//
//        const auto resultPartialFunc5 = partialApply(Just(1))(Just(0.5))(Just(std::string("Text")));
//        ASSERT_EQ(resultPartialFunc5.value(), "21.000000Text");
//
//        const auto resultPartialFunc6 = partialApply(Just(1), Just(0.5), Just(std::string("Text")));
//        ASSERT_EQ(resultPartialFunc6.value(), "21.000000Text");
//
//        const auto resultPartialFunc7 = partialApply.value()(1,3, std::string("Text"));
//        ASSERT_EQ(resultPartialFunc7, "126.000000Text");
//    }
//    {
//        const auto partialApply = Just([](int x, float f, const std::string& s){ return std::to_string(x*f*42) + s;});
//        const auto resultPartialNothing = partialApply(Nothing<int>());
//        const auto resultPartialFunc2 = resultPartialNothing(Just(0.5));
//        const auto resultPartialFunc3 = resultPartialFunc2(Just<std::string>("Text"));
//
//        ASSERT_FALSE(resultPartialFunc3.hasValue());
//    }
//    {
//        const auto func = [](int){ return;};
//        const auto partialApply = Nothing<decltype(func)>();
//        const auto resultPartialJust = partialApply(Nothing<int>());
//        ASSERT_FALSE(resultPartialJust.hasValue());
//    }
}