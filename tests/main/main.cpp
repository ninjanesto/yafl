/**
 * \brief       Yet Another Functional Library
 *
 * \copyright   2023, Ernesto Festas.
 *              Distributed under MIT license (See accompanying LICENSE file)
 */

#include <sstream>
#include <iostream>
#include <gtest/gtest.h>

/**
 * Main used for tests
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    const auto ret = RUN_ALL_TESTS();

    const auto& testInstance = ::testing::UnitTest::GetInstance();

    std::stringstream message;
    message << "msg: 'Test has finished', result:" << (testInstance->Passed() ? "PASS" : "FAIL") << ", binary:" << argv[0]
            << ", elapsed_time:" << testInstance->elapsed_time();

    if (testInstance->Passed()) {
        std::cout << message.str();
    } else {
        std::cerr << message.str();
    }

    return ret;
}
