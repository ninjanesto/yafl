/**
 * \file
 * \brief       Yet Another Functional Library
 *
 * \project     Critical TechWorks SA
 * \copyright   Critical TechWorks SA
 */

#include "yafl/Compose.h"
#include <string>
#include <gtest/gtest.h>

using namespace yafl;

TEST(ComposeTest, validate_function_composition) {
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
