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
#ifdef __unix__
    EXPECT_NO_THROW((MEMOCPP_THROW_IF_FALSE(0 == 0, std::runtime_error)));
#elif defined(_WIN32) || defined(_WIN64)
    EXPECT_NO_THROW(MEMOCPP_THROW_IF_FALSE(0 == 0, std::runtime_error));
#endif
}

TEST(Errors_cpp_test, core_except_throws_specified_exception_when_condition_fails)
{
#ifdef __unix__
    EXPECT_THROW((MEMOCPP_THROW_IF_FALSE(0 == 1, std::runtime_error)), std::runtime_error);
#elif defined(_WIN32) || defined(_WIN64)
    EXPECT_THROW(MEMOCPP_THROW_IF_FALSE(0 == 1, std::runtime_error), std::runtime_error);
#endif
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

TEST(Expected_test, can_have_either_value_or_error_of_any_types)
{
    using namespace memoc;

    enum class Errors {
        division_by_zero
    };

    auto divide = [](int a, int b) -> Expected<int, Errors> {
        if (b == 0) {
            return Errors::division_by_zero;
        }
        return a / b;
    };

    {
        auto result = divide(2, 1);
        EXPECT_TRUE(result);
        EXPECT_EQ(2, result.value());

        EXPECT_EQ(2, result.value_or(-1));
    }

    {
        auto result = divide(2, 0);
        EXPECT_FALSE(result);
        EXPECT_EQ(Errors::division_by_zero, result.error());

        EXPECT_EQ(-1, result.value_or(-1));
    }
}

TEST(Optional_test, can_have_either_value_or_not)
{
    using namespace memoc;

    auto whole_divide = [](int a, int b) -> Optional<int> {
        if (a % b != 0) {
            return Nullopt{};
        }
        return a / b;
    };

    {
        auto result = whole_divide(4, 2);
        EXPECT_TRUE(result);
        EXPECT_EQ(2, result.value());

        EXPECT_EQ(2, result.value_or(-1));
    }

    {
        auto result = whole_divide(3, 2);
        EXPECT_FALSE(result);

        EXPECT_EQ(-1, result.value_or(-1));
    }
}
