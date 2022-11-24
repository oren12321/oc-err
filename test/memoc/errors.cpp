#include <gtest/gtest.h>

#include <stdexcept>
#include <regex>

#include <memoc/errors.h>

TEST(Errors_test, core_expect_not_throwing_exception_when_condition_is_true)
{
    EXPECT_NO_THROW(MEMOC_THROW_IF_FALSE(0 == 0, std::runtime_error));
}

TEST(Errors_test, core_except_throws_specified_exception_when_condition_fails)
{
    EXPECT_THROW(MEMOC_THROW_IF_FALSE(0 == 1, std::runtime_error), std::runtime_error);
}

TEST(Errors_test, core_expect_throws_an_exception_with_specific_description)
{
    try {
        MEMOC_THROW_IF_FALSE(0 == 1, std::runtime_error, "some message with optional %d value", 0);
        FAIL();
    }
    catch (const std::runtime_error& ex) {
        const std::regex re("^'.+' failed on '.+' at line:[0-9]+@.+@.+ with message: .+$");
        EXPECT_TRUE(std::regex_match(ex.what(), re));
    }
}

TEST(Errors_cpp_test, core_expect_not_throwing_exception_when_condition_is_true)
{
    EXPECT_NO_THROW(MEMOCPP_THROW_IF_FALSE(0 == 0, std::runtime_error));
}

TEST(Errors_cpp_test, core_except_throws_specified_exception_when_condition_fails)
{
    EXPECT_THROW(MEMOCPP_THROW_IF_FALSE(0 == 1, std::runtime_error), std::runtime_error);
}

TEST(Errors_cpp_test, core_expect_throws_an_exception_with_specific_description)
{
    try {
        MEMOCPP_THROW_IF_FALSE(0 == 1, std::runtime_error, << "some message with optional " << 0 << " value");
        FAIL();
    }
    catch (const std::runtime_error& ex) {
        const std::regex re("^'.+' failed on '.+' at line:[0-9]+@.+@.+ with message: .+$");
        EXPECT_TRUE(std::regex_match(ex.what(), re));
    }
}

