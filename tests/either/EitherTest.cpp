/**
 * \brief       Yet Another Functional Library
 *
 * \copyright   2023, Ernesto Festas.
 *              Distributed under MIT license (See accompanying LICENSE file)
 */

#include "yafl/HOF.h"
#include "yafl/Either.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

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

template <typename E>
struct FixedErrorType {
    template <typename T>
    using Type = Either<E, T>;
};

template<typename T>
using NoErrorEither = Either<void, T>;

TYPED_TEST_SUITE(EitherTest, EitherTypes,);

TYPED_TEST(EitherTest, assertErrorCreateAValidEitherForAnyType) {
    Either<typename TypeParam::Error, typename TypeParam::Ok> either = createError<typename TypeParam::Error, typename TypeParam::Ok>();
    ASSERT_TRUE(either.isError());
}

TYPED_TEST(EitherTest, assertOkCreateAValidEitherForAnyType) {
    Either<typename TypeParam::Error, typename TypeParam::Ok> either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
    ASSERT_TRUE(either.isOk());
}

TYPED_TEST(EitherTest, assertCopyConstructor) {
    {
        const auto maybe = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
        const Either destination(maybe);
        ASSERT_EQ(maybe, destination);
    }
    {
        const auto maybe = createError<typename TypeParam::Error, typename TypeParam::Ok>();
        const Either destination(maybe);
        ASSERT_EQ(maybe, destination);
    }
}

TYPED_TEST(EitherTest, assertAssignmentOperator) {
    {
        const auto maybe = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
        auto destination = createError<typename TypeParam::Error, typename TypeParam::Ok>();
        destination = maybe;
        ASSERT_EQ(maybe, destination);
    }
    {
        const auto maybe = createError<typename TypeParam::Error, typename TypeParam::Ok>();
        auto destination = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
        destination = maybe;
        ASSERT_EQ(maybe, destination);
    }
}

TYPED_TEST(EitherTest, assertMoveConstructor) {
    {
        auto maybe = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
        const auto destination(std::move(maybe));
        EXPECT_TRUE(destination.isOk());
    }
    {
        auto maybe = createError<typename TypeParam::Error, typename TypeParam::Ok>();
        const auto destination(std::move(maybe));
        EXPECT_TRUE(destination.isError());
    }
}

TYPED_TEST(EitherTest, MoveAssignment) {
    {
        auto maybe = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
        auto destination = createError<typename TypeParam::Error, typename TypeParam::Ok>();
        destination = std::move(maybe);
        EXPECT_TRUE(destination.isOk());
    }
    {
        auto maybe = createError<typename TypeParam::Error, typename TypeParam::Ok>();
        auto destination = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
        destination = std::move(maybe);
        EXPECT_TRUE(destination.isError());
    }
}

TYPED_TEST(EitherTest, assertValueOr) {
    if constexpr (!std::is_void_v<typename TypeParam::Ok>) {
        Either<typename TypeParam::Error, typename TypeParam::Ok> eitherOk = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
        ASSERT_TRUE(eitherOk.isOk());
        ASSERT_EQ(eitherOk.value(), 0);
        ASSERT_EQ(eitherOk.valueOr(42), 0);
        Either<typename TypeParam::Error, typename TypeParam::Ok> eitherError = createError<typename TypeParam::Error, typename TypeParam::Ok>();
        ASSERT_TRUE(eitherError.isError());
        EXPECT_THAT([&eitherError]() { std::ignore = eitherError.value(); }, testing::Throws<std::runtime_error>());
        ASSERT_EQ(eitherError.valueOr(42), 42);
    }
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
            const auto resultVoid = functor::fmap([](){}, either);
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
            const auto resultVoid = functor::fmap([](const typename TypeParam::Ok&){}, either);
            ASSERT_TRUE(resultVoid.isOk());
        }
        {
            const auto either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto f = functor::fmap<FixedErrorType<typename TypeParam::Error>::template Type>([](const typename TypeParam::Ok&){});
            const auto resultVoid = f(either);
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
            const auto resultOk = either.bind([]() { return either::Ok<void, void>(); });
            ASSERT_TRUE(resultOk.isOk());
        }
        {
            const auto either = createOk<void, void>();
            const auto resultOk = monad::bind([]() { return either::Ok<void, void>(); }, either);
            ASSERT_TRUE(resultOk.isOk());
        }
        {
            const auto either = createOk<void, void>();
            const auto f = monad::bind<FixedErrorType<void>::template Type>([]() { return either::Ok<void, void>(); });
            const auto resultOk = f(either);
            ASSERT_TRUE(resultOk.isOk());
        }
        {
            const auto either = createOk<void, void>();
            const auto resultError = either.bind([]() { return either::Error<void, void>(); });
            ASSERT_TRUE(resultError.isError());
        }
        {
            const auto either = createError<void, void>();
            const auto resultError = either.bind([]() { return either::Ok<void, void>(); });
            ASSERT_TRUE(resultError.isError());
        }

        {
            const auto either = createError<void, void>();
            const auto resultError = either.bind([]() { return either::Error<void, void>(); });
            ASSERT_TRUE(resultError.isError());
        }
    }

    if constexpr (std::is_void_v<typename TypeParam::Ok> && !std::is_void_v<typename TypeParam::Error>) {
        {
            const auto either = createOk<typename TypeParam::Error, void>();
            const auto resultOk = either.bind([]() { return either::Ok<typename TypeParam::Error, void>(); });
            ASSERT_TRUE(resultOk.isOk());
        }
        {
            const auto either = createOk<typename TypeParam::Error, void>();
            const auto resultOk = monad::bind([]() { return either::Ok<typename TypeParam::Error, void>(); }, either);
            ASSERT_TRUE(resultOk.isOk());
        }
        {
            const auto either = createOk<typename TypeParam::Error, void>();
            const auto f = monad::bind<FixedErrorType<typename TypeParam::Error>::template Type>([]() { return either::Ok<typename TypeParam::Error, void>(); });
            const auto resultOk = f(either);
            ASSERT_TRUE(resultOk.isOk());
        }
        {
            const auto either = createOk<typename TypeParam::Error, void>();
            const auto resultError = either.bind([]() { return either::Error<typename TypeParam::Error, void>(42); });
            ASSERT_TRUE(resultError.isError());
        }
        {
            const auto either = createError<typename TypeParam::Error, void>();
            const auto resultError = either.bind([]() { return either::Ok<typename TypeParam::Error, void>(); });
            ASSERT_TRUE(resultError.isError());
        }
        {
            const auto either = createError<typename TypeParam::Error, void>();
            const auto resultError = either.bind([]() { return either::Error<typename TypeParam::Error, void>(42); });
            ASSERT_TRUE(resultError.isError());
        }
    }

    if constexpr (!std::is_void_v<typename TypeParam::Ok> && std::is_void_v<typename TypeParam::Error>) {
        {
            const auto either = createOk<void, typename TypeParam::Ok>();
            const auto resultOk = either.bind([](const typename TypeParam::Ok &) { return either::Ok<void, std::string>("42"); });
            ASSERT_TRUE(resultOk.isOk());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultOk.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createOk<void, typename TypeParam::Ok>();
            const auto resultOk = monad::bind([](const typename TypeParam::Ok &) { return either::Ok<void, std::string>("42"); }, either);
            ASSERT_TRUE(resultOk.isOk());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultOk.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createOk<void, typename TypeParam::Ok>();
            const auto f = monad::bind<FixedErrorType<void>::template Type>([](const typename TypeParam::Ok &) { return either::Ok<void, std::string>("42"); });
            const auto resultOk = f(either);
            ASSERT_TRUE(resultOk.isOk());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultOk.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createOk<void, typename TypeParam::Ok>();
            const auto resultError = either.bind([](const typename TypeParam::Ok &) { return either::Error<void, std::string>(); });
            ASSERT_TRUE(resultError.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultError.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createError<void, typename TypeParam::Ok>();
            const auto resultError = either.bind([](const typename TypeParam::Ok &) { return either::Ok<void, std::string>("42"); });
            ASSERT_TRUE(resultError.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultError.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createError<void, typename TypeParam::Ok>();
            const auto resultError = either.bind([](const typename TypeParam::Ok &) { return either::Error<void, std::string>(); });
            ASSERT_TRUE(resultError.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultError.value())>;
            ASSERT_TRUE(areSameType);
        }
    }

    if constexpr (!std::is_void_v<typename TypeParam::Ok> && !std::is_void_v<typename TypeParam::Error>) {
        {
            const auto either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultOk = either.bind([](const typename TypeParam::Ok&){ return either::Ok<typename TypeParam::Error, std::string>(std::string("42")); });
            ASSERT_TRUE(resultOk.isOk());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultOk.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultOk = monad::bind([](const typename TypeParam::Ok&){ return either::Ok<typename TypeParam::Error, std::string>(std::string("42")); }, either);
            ASSERT_TRUE(resultOk.isOk());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultOk.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto f = monad::bind<FixedErrorType<typename TypeParam::Error>::template Type>([](const typename TypeParam::Ok&){ return either::Ok<typename TypeParam::Error, std::string>(std::string("42")); });
            const auto resultOk = f(either);
            ASSERT_TRUE(resultOk.isOk());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultOk.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultError = either.bind([](const typename TypeParam::Ok &) { return either::Error<typename TypeParam::Error, std::string>(29); });
            ASSERT_TRUE(resultError.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultError.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createError<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultError = either.bind([](const typename TypeParam::Ok &) { return either::Ok<typename TypeParam::Error, std::string>("29"); });
            ASSERT_TRUE(resultError.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultError.value())>;
            ASSERT_TRUE(areSameType);
        }
        {
            const auto either = createError<typename TypeParam::Error, typename TypeParam::Ok>();
            const auto resultError = either.bind([](const typename TypeParam::Ok &) { return either::Error<typename TypeParam::Error, std::string>(29);
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
        EXPECT_THROW({std::ignore = either.value();}, std::runtime_error );
    }
}

TYPED_TEST(EitherTest, validateErrorThrowsExceptionWhenEitherIsValue) {
    if constexpr (!std::is_void_v<typename TypeParam::Error>) {
        const auto either = createOk<typename TypeParam::Error, typename TypeParam::Ok>();
        EXPECT_THROW({std::ignore = either.error();}, std::runtime_error );
    }
}

TEST(EitherTest, assertApply) {
    {
        const auto func = []() { return 42; };
        const auto funcOk = either::Ok<void>(func);
        const auto result = funcOk();
        ASSERT_EQ(result.value(), 42);
        const auto funcError = either::Error<void, decltype(func)>();
        const auto result2 = funcError();
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto func = []() { return; };
        const auto funcOk = either::Ok<void>(func);
        const auto result = funcOk();
        ASSERT_TRUE(result.isOk());
        const auto funcError = either::Error<void, decltype(func)>();
        const auto result2 = funcError();
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto multBy42 = either::Ok<void>([](int x) { return x * 42; });
        const auto result = multBy42(2);
        ASSERT_EQ(result.value(), 84);
    }
    {
        const auto func = []() { return 42; };
        const auto funcOk = either::Ok<int>(func);
        const auto result = funcOk();
        ASSERT_EQ(result.value(), 42);
        const auto funcError = either::Error<int, decltype(func)>(29);
        const auto result2 = funcError();
        ASSERT_TRUE(result2.isError());
        ASSERT_EQ(result2.error(), 29);
    }
    {
        const auto func = either::Ok<int>([]() { return; });
        const auto result = func();
        ASSERT_TRUE(result.isOk());
    }
    {
        const auto multBy42 = either::Ok<int>([](int) { return; });
        const auto result = multBy42(2);
        ASSERT_TRUE(result.isOk());
    }
    {
        const auto multBy42 = either::Ok<void>([](int x) { return x * 42; });
        const auto result = multBy42(either::Ok<void>(2));
        ASSERT_EQ(result.value(), 84);
        const auto result2 = multBy42(either::Error<void, int>());
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto multBy42 = either::Ok<int>([](int) { return; });
        const auto result = multBy42(either::Ok<int>(2));
        ASSERT_TRUE(result.isOk());
        EXPECT_THROW(multBy42(either::Error<int, int>(29)), std::runtime_error);
    }
    {
        const auto multBy42 = either::Ok<void>([](int) { return; });
        const auto result = multBy42(2);
        ASSERT_TRUE(result.isOk());
    }
    {
        const auto multBy42 = either::Ok<void>([](int) { return; });
        const auto result = multBy42(either::Ok<void>(2));
        ASSERT_TRUE(result.isOk());
        const auto result2 = multBy42(either::Error<void, int>());
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto func = [](int x) { return x * 42; };
        const auto applmultBy42 = either::Ok<int, decltype(func)>(func);
        const auto result = applmultBy42(2);
        ASSERT_EQ(result.value(), 84);
    }
    {
        const auto func = [](int x) { return x * 42; };
        const auto applmultBy42 = either::Ok<int, decltype(func)>(func);
        const auto result = applmultBy42(either::Ok<int, int>(2));
        ASSERT_EQ(result.value(), 84);
        EXPECT_THROW(applmultBy42(either::Error<int, int>(2)), std::runtime_error);
    }
    {
        const auto func = [](int x, int f) { return x * f; };
        const auto applmultBy42 = either::Ok<void, decltype(func)>(func);
        const auto result = applmultBy42(2)(2);
        ASSERT_EQ(result.value(), 4);
        const auto applmultBy42Error = either::Error<void, decltype(func)>();
        const auto result2 = applmultBy42Error(2)(2);
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto func = [](int x, int f) { return x * f; };
        const auto applmultBy42 = either::Ok<void, decltype(func)>(func);
        const auto result = applmultBy42(either::Ok<void, int>(2))(either::Ok<void, int>(2));
        ASSERT_EQ(result.value(), 4);
        const auto result2 = applmultBy42(either::Error<void, int>());
        ASSERT_TRUE(result2.isError());
        const auto result3 = result2(either::Error<void, int>());
        ASSERT_TRUE(result3.isError());
    }
    {
        const auto func = [](int x, int f) { return x * f; };
        const auto applmultBy42 = either::Error<void, decltype(func)>();
        const auto result = applmultBy42(either::Ok<void, int>(2))(2);
        ASSERT_TRUE(result.isError());
    }
    {
        const auto func = [](int x, int f) { return x * f; };
        const auto applmultBy42 = either::Error<void, decltype(func)>();
        const auto result = applmultBy42(either::Ok<void, int>(2))(either::Ok<void, int>(2));
        ASSERT_TRUE(result.isError());
        const auto result2 = applmultBy42(either::Error<void, int>())(either::Ok<void, int>(2));
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto func = [](const std::string &s, float f) {
            std::stringstream ss;
            ss << s << std::fixed << std::setprecision(2) << f * 42;
            return ss.str();
        };
        const auto applmultBy42 = either::Ok<int, decltype(func)>(func);
        const auto result = applmultBy42(either::Ok<int, std::string>("OlaOla"))(2.2);
        ASSERT_EQ(result.value(), "OlaOla92.40");
    }
    {
        const auto func = [](const std::string &s, float f) {
            std::stringstream ss;
            ss << s << std::fixed << std::setprecision(2) << f * 42;
            return ss.str();
        };
        const auto applmultBy42 = either::Ok<int, decltype(func)>(func);
        const auto result = applmultBy42("OlaOla")(2.2);
        ASSERT_EQ(result.value(), "OlaOla92.40");
    }
    {
        const auto func = [](const std::string &s, float f) {
            std::stringstream ss;
            ss << s << std::fixed << std::setprecision(2) << f * 42;
            return ss.str();
        };
        const auto applmultBy42 = either::Ok<int, decltype(func)>(func);
        const auto result = applmultBy42(either::Ok<int, std::string>("OlaOla"))(either::Ok<int, float>(2.2));
        ASSERT_EQ(result.value(), "OlaOla92.40");
        EXPECT_THROW(applmultBy42(either::Error<int, std::string>(2))(either::Ok<int, float>(2.2)), std::runtime_error);
        EXPECT_THROW(applmultBy42(either::Error<int, std::string>(2))(either::Error<int, float>(2)), std::runtime_error);
    }
    {
        const auto func = [](const std::string&, float) {return std::string();};
        const auto applmultBy42 = either::Error<int, decltype(func)>(2);
        const auto result = applmultBy42("OlaOla");
        ASSERT_TRUE(result.isError());
        const auto result2 = result(2.2);
        ASSERT_TRUE(result2.isError());
    }
    {
        const auto func = [](const std::string&, float) {return std::string();};
        const auto applmultBy42 = either::Error<int, decltype(func)>(2);
        const auto result = applmultBy42(either::Ok<int, std::string>("OlaOla"));
        ASSERT_TRUE(result.isError());
        const auto result2 = result(either::Ok<int, float>(2.2));
        ASSERT_TRUE(result2.isError());
    }
}


TEST(EitherTest, validate_kleisli_compose){
    {
        const auto f1 = [](int i) { return either::Ok<void>(i * 2);};
        const auto f2 = [](int i) { return either::Ok<void>(std::to_string(i * 2));};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(2).value(), "8");
    }
    {
        const auto f1 = [](const Either<void, int>& i) { return either::Ok<void>(i.value() * 2);};
        const auto f2 = [](int i) { return either::Ok<void>(std::to_string(i * 2));};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(either::Ok<void>(2)).value(), "8");
    }
    {
        const auto f1 = [](int i) { return either::Ok<std::string>(i * 2);};
        const auto f2 = [](int i) { return either::Ok<std::string>(std::to_string(i * 2));};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(2).value(), "8");
    }
    {
        const auto f1 = [](const Either<std::string, int>& i) { return either::Ok<std::string>(i.value() * 2);};
        const auto f2 = [](int i) { return either::Ok<std::string>(std::to_string(i * 2));};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(either::Ok<std::string>(2)).value(), "8");
    }
    {
        const auto f1 = [](int i) { return either::Ok<int>(i * 2);};
        const auto f2 = [](const Either<int, int>& i) { return either::Ok<int>(std::to_string(i.value() * 2));};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(2).value(), "8");
    }
    {
        const auto f1 = [](const Either<int, int>& i) { return either::Ok<int>(i.value() * 2);};
        const auto f2 = [](const Either<int, int>& i) { return either::Ok<int>(std::to_string(i.value() * 2));};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(either::Ok<int>(2)).value(), "8");
    }
    {
        const auto f1 = [](int) { return either::Error<void, int>();};
        const auto f2 = [](int i) { return either::Ok<void>(i * 2);};
        const auto f = compose(f1, f2);
        ASSERT_FALSE(f(2).isOk());
    }
    {
        const auto f1 = [](int i) { return either::Ok<void>(i * 2);};
        const auto f2 = [](int) { return either::Error<void, int>();};
        const auto f = compose(f1, f2);
        ASSERT_FALSE(f(2).isOk());
    }
    {
        const auto f1 = [](int) { return either::Error<void, void>();};
        const auto f2 = []() { return either::Ok<void, void>();};
        const auto f = compose(f1, f2);
        ASSERT_FALSE(f(2).isOk());
    }
    {
        const auto f1 = [](int) { return either::Ok<int, void>();};
        const auto f2 = []() { return either::Ok<int, void>();};
        const auto f = compose(f1, f2);
        ASSERT_TRUE(f(2).isOk());
    }
}


TEST(EitherTest, validate_lift) {
    {
        const auto funcNoArgRetVoid = []() { return;};
        const auto lifted = yafl::either::lift(funcNoArgRetVoid);
        const auto result = lifted();
        ASSERT_TRUE(result.isOk());
    }
    {
        const auto funcNoArgRetInt = []() { return 42;};
        const auto lifted = yafl::either::lift(funcNoArgRetInt);
        const auto result = lifted();
        ASSERT_TRUE(result.isOk());
        ASSERT_EQ(result.value(), 42);
    }
    {
        const auto funcOneArgRetVoid = [](int) { return;};
        const auto lifted = yafl::either::lift(funcOneArgRetVoid);
        const auto result = lifted(either::Ok<void>(2));
        ASSERT_TRUE(result.isOk());
        const auto result2 = lifted(either::Error<void, int>());
        ASSERT_FALSE(result2.isOk());
    }
    {
        const auto funcOneArgRetInt = [](int i) { return 42 * i;};
        const auto lifted = yafl::either::lift(funcOneArgRetInt);
        const auto result = lifted(either::Ok<void>(1));
        ASSERT_TRUE(result.isOk());
        ASSERT_EQ(result.value(), 42);
        const auto result2 = lifted(either::Error<void, int>());
        ASSERT_FALSE(result2.isOk());
    }
    {
        const auto funcMultiArgRetVoid = [](int, float, const std::string&) { return;};
        const auto lifted = yafl::either::lift(funcMultiArgRetVoid);
        const auto result = lifted(either::Ok<void>(2), either::Ok<void>(4.2), either::Ok<void, std::string>("dummy"));
        ASSERT_TRUE(result.isOk());
        const auto result2 = lifted(either::Error<void, int>(), either::Ok<void>(4.2), either::Ok<void, std::string>("dummy"));
        ASSERT_FALSE(result2.isOk());
        const auto result3 = lifted(either::Ok<void>(1), either::Error<void, float>(), either::Ok<void, std::string>("dummy"));
        ASSERT_FALSE(result3.isOk());
        const auto result4 = lifted(either::Ok<void>(1), either::Ok<void>(4.2), either::Error<void, std::string>());
        ASSERT_FALSE(result4.isOk());
    }
    {
        const auto funcMultiArgRetString = [](int i, int j, const std::string& s) { return s + std::to_string(i + j);};
        const auto lifted = yafl::either::lift(funcMultiArgRetString);
        const auto result = lifted(either::Ok<void>(2), either::Ok<void>(4), either::Ok<void, std::string>("dummy"));
        ASSERT_TRUE(result.isOk());
        ASSERT_EQ(result.value(), "dummy6");
        const auto result2 = lifted(either::Error<void, int>(), either::Ok<void>(4.2), either::Ok<void, std::string>("dummy"));
        ASSERT_FALSE(result2.isOk());
        const auto result3 = lifted(either::Ok<void>(1), either::Error<void, float>(), either::Ok<void, std::string>("dummy"));
        ASSERT_FALSE(result3.isOk());
        const auto result4 = lifted(either::Ok<void>(1), either::Ok<void>(4.2), either::Error<void, std::string>());
        ASSERT_FALSE(result4.isOk());
    }
}
