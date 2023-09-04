#include "Either.h"
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
    Either<typename TypeParam::Error, typename TypeParam::Ok> either = createOk<typename TypeParam::Error,
                                                                                typename TypeParam::Ok>();
    ASSERT_TRUE(either.isOk());
}

TYPED_TEST(EitherTest, assertFmapComputesCorrectType) {
    if constexpr (std::is_void_v<typename TypeParam::Ok>) {
        {
            const auto either = createOk<typename TypeParam::Error,
                                         typename TypeParam::Ok>();
            const auto resultVoid = either.fmap([](){});
            ASSERT_TRUE(resultVoid.isOk());
        }
        {
            const auto either = createOk<typename TypeParam::Error,
                                         typename TypeParam::Ok>();
            const auto resultType = either.fmap([](){return std::to_string(42);});
            ASSERT_TRUE(resultType.isOk());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultType.value())>;
            ASSERT_TRUE(areSameType);

        }
    } else {
        {
            const auto either = createOk<typename TypeParam::Error,
                                         typename TypeParam::Ok>();
            const auto resultVoid = either.fmap([](const typename TypeParam::Ok&){});
            ASSERT_TRUE(resultVoid.isOk());
        }
        {
            const auto either = createOk<typename TypeParam::Error,
                                         typename TypeParam::Ok>();
            const auto resultType = either.fmap([](const typename TypeParam::Ok&){return std::to_string(42);});
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultType.value())>;
            ASSERT_TRUE(areSameType);
        }
    }

    if constexpr (std::is_void_v<typename TypeParam::Error>) {
        if constexpr (std::is_void_v<typename TypeParam::Ok>) {
            const auto either = createError<typename TypeParam::Error,
                                            typename TypeParam::Ok>();
            const auto resultVoid = either.fmap([](){});
            ASSERT_TRUE(resultVoid.isError());
        } else {
            const auto either = createError<typename TypeParam::Error,
                                            typename TypeParam::Ok>();
            const auto resultType = either.fmap([](const typename TypeParam::Ok&){return std::to_string(42);});
            ASSERT_TRUE(resultType.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultType.value())>;
            ASSERT_TRUE(areSameType);
        }
    } else {
        if constexpr (std::is_void_v<typename TypeParam::Ok>) {
            const auto either = createError<typename TypeParam::Error,
                                            typename TypeParam::Ok>();
            const auto resultVoid = either.fmap([](){});
            ASSERT_TRUE(resultVoid.isError());
        } else {
            const auto either = createError<typename TypeParam::Error,
                                            typename TypeParam::Ok>();
            const auto resultType = either.fmap([](const typename TypeParam::Ok&){return std::to_string(42);});
            ASSERT_TRUE(resultType.isError());
            constexpr const auto areSameType = std::is_same_v<std::string, decltype(resultType.value())>;
            ASSERT_TRUE(areSameType);
        }
    }
}
