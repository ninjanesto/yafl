/**
 * \brief       Yet Another Functional Library
 *
 * \copyright   2023, Ernesto Festas.
 *              Distributed under MIT license (See accompanying LICENSE file)
 */

#include "yafl/HOF.h"
#include <string>
#include <gtest/gtest.h>

using namespace yafl;

template<typename T>
int xpto(const T&) {return 0;}

TEST(HOFTest, validate_function_composition) {
    {
        const auto f1 = []() {};
        const auto f2 = []() {};
        const auto f = compose(f1, f2);
        ASSERT_NO_FATAL_FAILURE(f());
    }
    {
        const auto f1 = [](int) {};
        const auto f2 = []() {};
        const auto f = compose(f1, f2);
        ASSERT_NO_FATAL_FAILURE(f(2));
    }
    {
        const auto f1 = []() { return 42;};
        const auto f2 = [](int) {};
        const auto f = compose(f1, f2);
        ASSERT_NO_FATAL_FAILURE(f());
    }
    {
        const auto f1 = [](int i ) { return 42 * i;};
        const auto f2 = [](int i) { return i * 2;};
        const auto f = compose(f1, f2);
        ASSERT_EQ(f(2), 168);
    }
}

TEST(HOFTest, validate_compose_id) {
    {
        const auto f1 = [](int i ) { return 42 * i;};
        const auto f = compose(id<int>, f1);
        ASSERT_EQ(f(2), 84);
    }
    {
        const auto f1 = [](const std::string& ) { return 42;};
        const auto f = compose(f1, id<int>);
        ASSERT_EQ(f("2"), 42);
    }
}

TEST(HOFTest, validate_compose_constf) {
    {
        const auto f1 = [](int i) { return 2 * i;};
        const auto ff = curry(constf<int, int>);
        const auto f = compose(ff(42), f1);
        ASSERT_EQ(f(42), 84);
    }
    {
        const auto f1 = [](int i) { return 2 * i;};
        const std::function<int(int)> ff = constf<int>(42);
        const auto f = compose(ff, f1);
        ASSERT_EQ(f(123), 84);
    }
    {
        const auto f1 = [](const std::string&) { return 42;};
        const auto f = compose(f1, curry(constf<int, int>));
        ASSERT_EQ(f("2")(2), 42);
    }
    {
        const auto f1 = [](const std::string& ) { return 24;};
        const auto f = compose(f1, constf<int>(42));
        ASSERT_EQ(f("2"), 42);
    }
}

TEST(HOFTest, validate_curry) {
    {
        const auto func = [](){ return 21*2; };
        const auto result = yafl::curry(func);
        ASSERT_EQ(result, 42);
    }
    {
        const auto func = [](int i){ return i*2; };
        const auto curried_func = yafl::curry(func);
        ASSERT_EQ(curried_func(21), 42);
    }
    {
        const auto func = [](int i, float f){ return i*f*2; };
        const auto curried_func = yafl::curry(func);
        ASSERT_EQ(curried_func(21)(2), 84);
    }
    {
        bool void_result = false;
        const auto func = [&void_result](int, float){ void_result = true; return; };
        const auto curried_func = yafl::curry(func);
        curried_func(21)(2);
        ASSERT_EQ(void_result, true);
    }
}

TEST(HOFTest, validate_uncurry) {
    {
        const auto func = [](int i){ return i*2; };
        const auto curried_func = yafl::curry(func);
        ASSERT_EQ(curried_func(21), 42);
        const auto uncurried_func = yafl::uncurry(curried_func);
        ASSERT_EQ(uncurried_func(21), 42);
    }
    {
        const auto func = [](int i, float f){ return i*f*2; };
        const auto curried_func = yafl::curry(func);
        ASSERT_EQ(curried_func(21)(2), 84);
        const auto uncurried_func = yafl::uncurry(curried_func);
        ASSERT_EQ(uncurried_func(21, 2), 84);
    }
    {
        const auto func = [](int i, float f){ return i*f*2; };
        const auto curried_func = yafl::curry(func);
        ASSERT_EQ(curried_func(21)(2), 84);
        const std::function<float (int, float)> uncurried_func = yafl::uncurry(curried_func);
        ASSERT_EQ(uncurried_func(21, 2), 84);
        const auto curried_func2 = yafl::curry(uncurried_func);
        ASSERT_EQ(curried_func2(21)(2), 84);
    }
    {
        bool void_result = false;
        const auto func = [&void_result](int, float){ void_result = true; return; };
        const auto curried_func = yafl::curry(func);
        const auto uncurried_func = yafl::uncurry(curried_func);
        uncurried_func(21, 2);
        ASSERT_EQ(void_result, true);
    }
}

TEST(HOFTest, validate_partial_application) {
    const auto func = [](int i, int j, const std::string& s){ return s + std::to_string(i*j); };
    {
        const auto partial0 = yafl::partial(func);
        ASSERT_EQ(partial0(2, 4, "ola"), "ola8");
    }
    {
        const auto partial1 = yafl::partial(func, 2);
        ASSERT_EQ(partial1(4, "ola"), "ola8");
    }
    {
        const auto partial2 = yafl::partial(func, 2, 4);
        ASSERT_EQ(partial2("ola"), "ola8");
    }
    {
        const auto partial3 = yafl::partial(func, 2, 4, "ola");
        ASSERT_EQ(partial3(), "ola8");
    }
    {
        const std::function<std::string(int, const std::string&)> partial1 = yafl::partial(func, 2);
        const auto partial3 = yafl::partial(partial1, 4, "ola");
        ASSERT_EQ(partial3(), "ola8");
    }
    {
        const auto func2 = [](){ return 42; };
        const auto result = yafl::partial(func2);
        ASSERT_EQ(result, 42);
    }
}