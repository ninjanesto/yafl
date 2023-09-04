/**
 * \file
 * \brief       Yet Another Functional Library
 *
 * \project     Critical TechWorks SA
 * \copyright   Critical TechWorks SA
 */

#include "yafl/Compose.h"
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
        const auto func = []() { return 42; };
        const auto funcOk = Ok<void>(func);
        const auto result = funcOk();
        ASSERT_EQ(result.value(), 42);
        const auto funcError = Error<void, decltype(func)>();
        const auto result2 = funcError();
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto func = []() { return; };
        const auto funcOk = Ok<void>(func);
        const auto result = funcOk();
        ASSERT_TRUE(result.isOk());
        const auto funcError = Error<void, decltype(func)>();
        const auto result2 = funcError();
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto multBy42 = Ok<void>([](int x) { return x * 42; });
        const auto result = multBy42(2);
        ASSERT_EQ(result.value(), 84);
    }
    {
        const auto func = []() { return 42; };
        const auto funcOk = Ok<int>(func);
        const auto result = funcOk();
        ASSERT_EQ(result.value(), 42);
        const auto funcError = Error<int, decltype(func)>(29);
        const auto result2 = funcError();
        ASSERT_TRUE(result2.isError());
        ASSERT_EQ(result2.error(), 29);
    }
    {
        const auto func = Ok<int>([]() { return; });
        const auto result = func();
        ASSERT_TRUE(result.isOk());
    }
    {
        const auto multBy42 = Ok<int>([](int) { return; });
        const auto result = multBy42(2);
        ASSERT_TRUE(result.isOk());
    }
    {
        const auto multBy42 = Ok<void>([](int x) { return x * 42; });
        const auto result = multBy42(Ok<void>(2));
        ASSERT_EQ(result.value(), 84);
        const auto result2 = multBy42(Error<void, int>());
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto multBy42 = Ok<int>([](int) { return; });
        const auto result = multBy42(Ok<int>(2));
        ASSERT_TRUE(result.isOk());
        const auto result2 = multBy42(Error<int, int>(29));
        ASSERT_TRUE(result2.isError());
        ASSERT_EQ(result2.error(), 29);
    }
    {
        const auto multBy42 = Ok<void>([](int) { return; });
        const auto result = multBy42(2);
        ASSERT_TRUE(result.isOk());
    }
    {
        const auto multBy42 = Ok<void>([](int) { return; });
        const auto result = multBy42(Ok<void>(2));
        ASSERT_TRUE(result.isOk());
        const auto result2 = multBy42(Error<void, int>());
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto func = [](int x) { return x * 42; };
        const auto applmultBy42 = Ok<int, decltype(func)>(func);
        const auto result = applmultBy42(2);
        ASSERT_EQ(result.value(), 84);
    }
    {
        const auto func = [](int x) { return x * 42; };
        const auto applmultBy42 = Ok<int, decltype(func)>(func);
        const auto result = applmultBy42(Ok<int, int>(2));
        ASSERT_EQ(result.value(), 84);
        const auto result2 = applmultBy42(Error<int, int>(2));
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto func = [](int x, int f) { return x * f; };
        const auto applmultBy42 = Ok<void, decltype(func)>(func);
        const auto result = applmultBy42(2)(2);
        ASSERT_EQ(result.value(), 4);
        const auto applmultBy42Error = Error<void, decltype(func)>();
        const auto result2 = applmultBy42Error(2)(2);
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto func = [](int x, int f) { return x * f; };
        const auto applmultBy42 = Ok<void, decltype(func)>(func);
        const auto result = applmultBy42(Ok<void, int>(2))(Ok<void, int>(2));
        ASSERT_EQ(result.value(), 4);
        const auto result2 = applmultBy42(Error<void, int>());
        ASSERT_TRUE(result2.isError());
        const auto result3 = result2(Error<void, int>());
        ASSERT_TRUE(result3.isError());
    }
    {
        const auto func = [](int x, int f) { return x * f; };
        const auto applmultBy42 = Error<void, decltype(func)>();
        const auto result = applmultBy42(Ok<void, int>(2))(2);
        ASSERT_TRUE(result.isError());
    }
    {
        const auto func = [](int x, int f) { return x * f; };
        const auto applmultBy42 = Error<void, decltype(func)>();
        const auto result = applmultBy42(Ok<void, int>(2))(Ok<void, int>(2));
        ASSERT_TRUE(result.isError());
        const auto result2 = applmultBy42(Error<void, int>())(Ok<void, int>(2));
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto func = [](const std::string &s, float f) {
            std::stringstream ss;
            ss << s << std::fixed << std::setprecision(2) << f * 42;
            return ss.str();
        };
        const auto applmultBy42 = Ok<int, decltype(func)>(func);
        const auto result = applmultBy42(Ok<int, std::string>("OlaOla"))(2.2);
        ASSERT_EQ(result.value(), "OlaOla92.40");
    }
    {
        const auto func = [](const std::string &s, float f) {
            std::stringstream ss;
            ss << s << std::fixed << std::setprecision(2) << f * 42;
            return ss.str();
        };
        const auto applmultBy42 = Ok<int, decltype(func)>(func);
        const auto result = applmultBy42("OlaOla")(2.2);
        ASSERT_EQ(result.value(), "OlaOla92.40");
    }
    {
        const auto func = [](const std::string &s, float f) {
            std::stringstream ss;
            ss << s << std::fixed << std::setprecision(2) << f * 42;
            return ss.str();
        };
        const auto applmultBy42 = Ok<int, decltype(func)>(func);
        const auto result = applmultBy42(Ok<int, std::string>("OlaOla"))(Ok<int, float>(2.2));
        ASSERT_EQ(result.value(), "OlaOla92.40");
        const auto result2 = applmultBy42(Error<int, std::string>(2))(Ok<int, float>(2.2));
        ASSERT_TRUE(result2.isError());
        const auto result3 = applmultBy42(Error<int, std::string>(2))(Error<int, float>(2));
        ASSERT_TRUE(result3.isError());
    }
    {
        const auto func = [](const std::string&, float) {return std::string();};
        const auto applmultBy42 = Error<int, decltype(func)>(2);
        const auto result = applmultBy42("OlaOla");
        ASSERT_TRUE(result.isError());
        const auto result2 = result(2.2);
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto func = [](const std::string&, float) {return std::string();};
        const auto applmultBy42 = Error<int, decltype(func)>(2);
        const auto result = applmultBy42(Ok<int, std::string>("OlaOla"));
        ASSERT_TRUE(result.isError());
        const auto result2 = result(Ok<int, float>(2.2));
        ASSERT_TRUE(result2.isError());
    }
}


TEST(EitherTest, validate_kleisli_compose){
    {
        const auto f1 = [](int i) { return Ok<int>(i*2);};
        const auto f2 = [](int i) { return i * 2;};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(2).value(), 8);
    }
    {
        const auto f1 = [](int i) { return Ok<int>(i*2);};
        const auto f2 = [](int i) { return Ok<int>(std::to_string(i * 2));};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(2).value(), "8");
    }
    {
        const auto f1 = [](int) { return Ok<std::string, void>();};
        const auto f2 = []() { return 2;};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(2).value(), 2);
    }
    {
        const auto f1 = [](int) { return Ok<std::string, void>();};
        const auto f2 = []() { return Ok<std::string, std::string>("2");};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(2).value(), "2");
    }
    {
        const auto f1 = [](int) { return Error<int, int>(2);};
        const auto f2 = [](int i) { return i * 2;};
        const auto f = compose(f1, f2);
        ASSERT_FALSE(f(2).isOk());
    }
    {
        const auto f1 = [](int) { return Error<std::string, int>("2");};
        const auto f2 = [](int i) { return Ok<std::string, int>(i * 2);};
        const auto f = compose(f1, f2);
        ASSERT_FALSE(f(2).isOk());
    }
    {
        const auto f1 = [](int i) { return Ok<void, int>(i * 2);};
        const auto f2 = [](int) { return Error<void, int>();};
        const auto f = compose(f1, f2);
        ASSERT_FALSE(f(2).isOk());
    }
    {
        const auto f1 = [](int) { return Error<void, void>();};
        const auto f2 = []() { return 2;};
        const auto f = compose(f1, f2);
        ASSERT_FALSE(f(2).isOk());
    }
    {
        const auto f1 = [](int) { return Error<void, void>();};
        const auto f2 = []() { return Ok<void, void>();};
        const auto f = compose(f1, f2);
        ASSERT_FALSE(f(2).isOk());
    }
}


TEST(EitherTest, validate_lift) {
    {
        const auto funcNoArgRetVoid = []() { return;};
        const auto lifted = yafl::lift<Either>(funcNoArgRetVoid);
        const auto result = lifted();
        ASSERT_TRUE(result.isOk());
    }
    {
        const auto funcNoArgRetInt = []() { return 42;};
        const auto lifted = yafl::lift<Either>(funcNoArgRetInt);
        const auto result = lifted();
        ASSERT_TRUE(result.isOk());
        ASSERT_EQ(result.value(), 42);
    }
    {
        const auto funcOneArgRetVoid = [](int) { return;};
        const auto lifted = yafl::lift<Either>(funcOneArgRetVoid);
        const auto result = lifted(yafl::Ok<void>(2));
        ASSERT_TRUE(result.isOk());
        const auto result2 = lifted(yafl::Error<void, int>());
        ASSERT_FALSE(result2.isOk());
    }
    {
        const auto funcOneArgRetInt = [](int i) { return 42 * i;};
        const auto lifted = yafl::lift<Either>(funcOneArgRetInt);
        const auto result = lifted(yafl::Ok<void>(1));
        ASSERT_TRUE(result.isOk());
        ASSERT_EQ(result.value(), 42);
        const auto result2 = lifted(yafl::Error<void, int>());
        ASSERT_FALSE(result2.isOk());
    }
    {
        const auto funcMultiArgRetVoid = [](int, float, const std::string&) { return;};
        const auto lifted = yafl::lift<Either>(funcMultiArgRetVoid);
        const auto result = lifted(yafl::Ok<void>(2), yafl::Ok<void>(4.2), yafl::Ok<void, std::string>("dummy"));
        ASSERT_TRUE(result.isOk());
        const auto result2 = lifted(yafl::Error<void, int>(), yafl::Ok<void>(4.2), yafl::Ok<void, std::string>("dummy"));
        ASSERT_FALSE(result2.isOk());
        const auto result3 = lifted(yafl::Ok<void>(1), yafl::Error<void, float>(), yafl::Ok<void, std::string>("dummy"));
        ASSERT_FALSE(result3.isOk());
        const auto result4 = lifted(yafl::Ok<void>(1), yafl::Ok<void>(4.2), yafl::Error<void, std::string>());
        ASSERT_FALSE(result4.isOk());
    }
    {
        const auto funcMultiArgRetString = [](int i, int j, const std::string& s) { return s + std::to_string(i + j);};
        const auto lifted = yafl::lift<Either>(funcMultiArgRetString);
        const auto result = lifted(yafl::Ok<void>(2), yafl::Ok<void>(4), yafl::Ok<void, std::string>("dummy"));
        ASSERT_TRUE(result.isOk());
        ASSERT_EQ(result.value(), "dummy6");
        const auto result2 = lifted(yafl::Error<void, int>(), yafl::Ok<void>(4.2), yafl::Ok<void, std::string>("dummy"));
        ASSERT_FALSE(result2.isOk());
        const auto result3 = lifted(yafl::Ok<void>(1), yafl::Error<void, float>(), yafl::Ok<void, std::string>("dummy"));
        ASSERT_FALSE(result3.isOk());
        const auto result4 = lifted(yafl::Ok<void>(1), yafl::Ok<void>(4.2), yafl::Error<void, std::string>());
        ASSERT_FALSE(result4.isOk());
    }
}
