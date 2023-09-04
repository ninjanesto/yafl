/**
 * \brief       Yet Another Functional Library
 *
 * \copyright   2023, Ernesto Festas.
 *              Distributed under MIT license (See accompanying LICENSE file)
 */

#include "yafl/HOF.h"
#include "yafl/Either.h"
#include "yafl/Maybe.h"
#include "yafl/HOF.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST(LawsTest, validateFunctionCompositionIsAssociative) {
    struct Xpto {
        std::string value;
    };
    {
        const auto f1 = [](int i) { return i * 42; };
        const auto f2 = [](int i) { return std::to_string(i); };
        const auto f3 = [](const std::string &s) { return Xpto{s}; };

        const auto comp_f1_f2 = yafl::compose(f1, f2);
        const auto threeway = yafl::compose(comp_f1_f2, f3);

        ASSERT_EQ(threeway(1).value, "42");
    }
    {
        const auto f1 =[](int i) { return i*42;};
        const auto f2 =[](int i) { return std::to_string(i);};
        const auto f3 =[](const std::string& s) { return Xpto{ s };};

        const auto comp_f2_f3 = yafl::compose(f2, f3);
        const auto threeway = yafl::compose(f1, comp_f2_f3);

        ASSERT_EQ(threeway(1).value, "42");
    }
}

TEST(LawsTest, validatePartialApplication) {
    struct Xpto { std::string value;};

    const auto f = [](int, int, float, const std::string&, const Xpto&) {return 42;};

    const auto partial_3arg_app = yafl::partial(f, 1, 2, 3.14);
    const auto result = partial_3arg_app("s", Xpto{""});

    ASSERT_EQ(result, 42);

    const std::function<int(float f, const std::string&, const Xpto&)> partial_2arg_app = yafl::partial(f, 1, 2);
    // Here we need to explicitly declare the correct type
    const auto partial_4arg_app = yafl::partial(partial_2arg_app, 3.14, "");
    const auto result2 = partial_4arg_app(Xpto{""});
    ASSERT_EQ(result2, 42);
}

int function1(int, float, const std::string&) { return 42;}

TEST(LawsTest, validateCurryUncurry) {
    {
        const auto curried_function1 = yafl::curry(function1);
        const auto curried_function1_with_arg1_and_arg2 = curried_function1(42)(3.14);
        const auto result = curried_function1_with_arg1_and_arg2("dummy");
        ASSERT_EQ(result, 42);

        const auto uncurried_function1 = yafl::uncurry(curried_function1);
        const auto result2 = uncurried_function1(42, 3.14, "dummy");
        ASSERT_EQ(result2, 42);
    }
    {
        const auto curried_function1 = yafl::curry([](int, int, int ,int){ return 42;});
        const auto curried_function1_with_arg1 = curried_function1(1);
        const auto result = curried_function1_with_arg1(2)(3)(4);
        ASSERT_EQ(result, 42);

        const auto uncurried_function1 = yafl::uncurry(curried_function1);
        const auto result2 = uncurried_function1(1, 2, 3, 4);
        ASSERT_EQ(result2, 42);
    }
}

TEST(LawsTest, validateIdConstf) {
    const auto v = yafl::id(42);
    ASSERT_EQ(v, 42);

    const auto ff = yafl::constf(42);
    ASSERT_EQ(ff(), 42);
    ASSERT_EQ(ff(""), 42);
    ASSERT_EQ(ff(1,"",3), 42);
}

TEST(LawsTest, validateFunctor) {
    {
        const std::unique_ptr<yafl::core::Functor<yafl::Maybe, int>> functor =
            std::make_unique<yafl::Maybe<int>>(yafl::maybe::Just(2));
        const auto result = functor->fmap([](int i) { return i * 21; });
        ASSERT_EQ(result.value(), 42);
    }
    {
        const std::unique_ptr<yafl::core::Functor<yafl::Either, int, int>> functor =
                std::make_unique<yafl::Either<int, int>>(yafl::either::Ok<int>(2));
        const auto result = functor->fmap([](int i) { return i * 21; });
        ASSERT_EQ(result.value(), 42);
    }
}

TEST(LawsTest, validateFunctorPreserveIndentityMorphism) {
    {
        const auto v = yafl::maybe::Just(420);
        const auto r2 = yafl::id(v);

        ASSERT_EQ(v.fmap(yafl::id<int>), r2);
        ASSERT_EQ(yafl::functor::fmap(yafl::id<int>, v), r2);
    }
    {
        const auto v = yafl::either::Ok<std::string>(420);
        const auto r2 = yafl::id(v);

        ASSERT_EQ(v.fmap(yafl::id<int>), r2);
        ASSERT_EQ(yafl::functor::fmap(&yafl::id<int>, v), r2);
    }
}

TEST(LawsTest, validateFunctorsPreserveCompositionOfMorphisms) {
    {
        const auto f1 = [](int i) { return std::to_string(i);};
        const auto f2 = [](const std::string& s) { return s + "dummy";};

        const auto m = yafl::maybe::Just(42);

        const auto compose = yafl::compose(f1, f2);
        const auto fmap_compose = yafl::functor::fmap<yafl::Maybe>(compose);

        const auto compose2 = yafl::compose(yafl::functor::fmap<yafl::Maybe>(f1), yafl::functor::fmap<yafl::Maybe>(f2));
        const auto result2 = compose2(m);

        ASSERT_EQ(fmap_compose(m), result2);
        ASSERT_EQ(m.fmap(compose), result2);
        ASSERT_EQ(yafl::functor::fmap(compose, m), result2);
        ASSERT_EQ(m.fmap(f1).fmap(f2), result2);
    }
    {
        const auto f1 = [](int i) { return std::to_string(i);};
        const auto f2 = [](const std::string& s) { return s + "dummy";};

        const auto m = yafl::either::Ok<std::string>(42);

        const auto compose = yafl::compose(f1, f2);
        const auto fmap_compose = yafl::functor::fmap<yafl::type::FixedErrorType<std::string>::template Type>(compose);

        const auto compose2 = yafl::compose(yafl::functor::fmap<yafl::type::FixedErrorType<std::string>::template Type>(f1),
                                            yafl::functor::fmap<yafl::type::FixedErrorType<std::string>::template Type>(f2));
        const auto result2 = compose2(m);

        ASSERT_EQ(fmap_compose(m), result2);
        ASSERT_EQ(m.fmap(compose), result2);
        ASSERT_EQ(yafl::functor::fmap(compose, m), result2);
        ASSERT_EQ(m.fmap(f1).fmap(f2), result2);
    }
}

TEST(LawsTest, validateApplicative) {
    {
        const std::unique_ptr<yafl::core::Applicative<yafl::Maybe, std::function<int(int)>>> applicative =
                std::make_unique<yafl::Maybe<std::function<int(int)>>>(yafl::maybe::Just<std::function<int(int)>>([](int i){ return i;}));
        const auto result = (*applicative)(42);
        ASSERT_EQ(result.value(), 42);
    }
    {
        const std::unique_ptr<yafl::core::Applicative<yafl::Either, int, std::function<int(int)>>> applicative =
                std::make_unique<yafl::Either<int, std::function<int(int)>>>(yafl::either::Ok<int, std::function<int(int)>>([](int i){ return i;}));
        const auto result = (*applicative)(42);
        ASSERT_EQ(result.value(), 42);
    }
}

TEST(LawsTest, validateApplicativeIdentity) {
    {
        const auto v = yafl::maybe::Just(42);
        const auto ap = yafl::maybe::Just(&yafl::id<int>);

        ASSERT_EQ(ap(42), v);
        ASSERT_EQ(ap(v), v);
        ASSERT_EQ(ap(yafl::Maybe(v)), v);
        ASSERT_EQ(ap(yafl::maybe::Just(42)), v);
    }
    {
        const auto v = yafl::either::Ok<std::string>(42);
        const auto ap = yafl::either::Ok<std::string>(&yafl::id<int>);

        ASSERT_EQ(ap(42), v);
        ASSERT_EQ(ap(v), v);
        ASSERT_EQ(ap(yafl::Either<std::string, int>(v)), v);
        ASSERT_EQ(ap(yafl::either::Ok<std::string>(42)), v);
    }
}

TEST(LawsTest, validateApplicativeComposition) {
    {
        const auto lf = [](int i){ return 42 * i;};
        const auto rf = [](int i){ return std::to_string(i);};
        const auto ap = yafl::maybe::Just(&yafl::compose<std::function<int(int)>,
                std::function<std::string(int)>>);
        const auto ap_result = ap(lf)(rf)(2);
        const auto ap_result2 = ap(yafl::maybe::Just(std::function(lf)))
                                  (yafl::maybe::Just(std::function(rf)))
                                  (yafl::maybe::Just(2));

        const auto ap2 = yafl::maybe::Just(std::function(lf))(2);
        const auto ap2_result = yafl::maybe::Just(std::function(rf))(ap2);

        ASSERT_EQ(ap_result, ap2_result);
        ASSERT_EQ(ap_result2, ap2_result);
    }
    {
        const auto lf = [](int i){ return 42 * i;};
        const auto rf = [](int i){ return std::to_string(i);};
        const auto ap = yafl::either::Ok<std::string>(&yafl::compose<std::function<int(int)>,
                std::function<std::string(int)>>);
        const auto ap_result = ap(lf)(rf)(2);
        const auto ap_result2 = ap(yafl::either::Ok<std::string>(std::function(lf)))
                                  (yafl::either::Ok<std::string>(std::function(rf)))
                                  (yafl::either::Ok<std::string>(2));

        const auto ap2 = yafl::either::Ok<std::string>(std::function(lf))(2);
        const auto ap2_result = yafl::either::Ok<std::string>(std::function(rf))(ap2);

        ASSERT_EQ(ap_result, ap2_result);
        ASSERT_EQ(ap_result2, ap2_result);
    }
}

TEST(LawsTest, validateApplicativeHomomorphism) {
    {
        const auto f = [](int i){ return 42 * i;};
        const auto ap = yafl::maybe::Just(f);

        ASSERT_EQ(ap(2), yafl::maybe::Just(f(2)));
        ASSERT_EQ(ap(yafl::maybe::Just(2)), yafl::maybe::Just(f(2)));
    }
    {
        const auto f = [](int i){ return 42 * i;};
        const auto ap = yafl::either::Ok<std::string>(f);

        ASSERT_EQ(ap(2), yafl::either::Ok<std::string>(f(2)));
        ASSERT_EQ(ap(yafl::either::Ok<std::string>(2)), yafl::either::Ok<std::string>(f(2)));
    }
}

TEST(LawsTest, validateApplicativeInterchange) {
    {
        const auto f = [](int i){ return 42 * i;};
        const auto ap = yafl::maybe::Just(f);

        const auto ap2 = yafl::maybe::Just([](auto func){ return func(2);});

        ASSERT_EQ(ap(yafl::maybe::Just(2)), ap2(f));
    }
    {
        const auto f = [](int i){ return 42 * i;};
        const auto ap = yafl::either::Ok<std::string>(f);

        const auto ap2 = yafl::either::Ok<std::string>([](auto func){ return func(2);});

        ASSERT_EQ(ap(yafl::either::Ok<std::string>(2)), ap2(f));
    }
}

TEST(LawsTest, validateMonad) {
    {
        const std::unique_ptr<yafl::core::Monad<yafl::Maybe, int>> monad =
                std::make_unique<yafl::Maybe<int>>(yafl::maybe::Just(420));
        const auto result = monad->bind([](int){ return yafl::Maybe<int>::Just(42);});
        ASSERT_EQ(result.value(), 42);
    }
    {
        const std::unique_ptr<yafl::core::Monad<yafl::Either, std::string, int>> monad =
                std::make_unique<yafl::Either<std::string, int>>(yafl::either::Ok<std::string>(420));
        const auto result = monad->bind([](int){ return yafl::Either<std::string, int>::Ok(42);});
        ASSERT_EQ(result.value(), 42);
    }
}

TEST(LawsTest, validateMonadLeftIdentity) {
    {
        const auto func = [](int i){ return yafl::maybe::Just(i*2);};
        const auto v = yafl::maybe::Just(42);
        const auto result = v.bind(func);
        const auto result2 = func(42);
        ASSERT_EQ(result, result2);
        ASSERT_EQ(yafl::monad::bind(func, v), func(42));
    }
    {
        const auto func = [](int i){ return yafl::either::Ok<std::string>(i*2);};
        const auto v = yafl::either::Ok<std::string>(42);
        const auto result = v.bind(func);
        const auto result2 = func(42);
        ASSERT_EQ(result, result2);
        ASSERT_EQ(yafl::monad::bind(func, v), func(42));
    }
}

TEST(LawsTest, validateMonadRightIdentity) {
    {
        const auto v = yafl::maybe::Just(42);
        const auto mreturn = [](int arg) {
            return yafl::maybe::Just(arg);
        };
        ASSERT_EQ(v.bind(mreturn), v);
    }
    {
        const auto v = yafl::either::Ok<void>(42);
        const auto mreturn = [](int arg) {
            return yafl::either::Ok<void>(arg);
        };
        ASSERT_EQ(v.bind(mreturn), v);
    }
}

TEST(LawsTest, validateMonadAssociativity) {
    {
        const auto v = yafl::maybe::Just(42);
        const auto f = [](int i) { return yafl::maybe::Just(2 * i);};
        const auto g = [](int i) { return yafl::maybe::Just(6 + i);};

        const auto lresult = v.bind(f).bind(g).value();
        const auto rresult = v.bind([f,g](int i) { return f(i).bind(g);}).value();
        ASSERT_EQ(lresult, rresult);
    }
    {
        const auto v = yafl::either::Ok<void>(42);
        const auto f = [](int i) { return yafl::either::Ok<void>(2 * i);};
        const auto g = [](int i) { return yafl::either::Ok<void>(6 + i);};

        const auto lresult = v.bind(f).bind(g).value();
        const auto rresult = v.bind([f,g](int i) { return f(i).bind(g);}).value();
        ASSERT_EQ(lresult, rresult);
    }
}