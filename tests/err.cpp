#include <gtest/gtest.h>

#include <stdexcept>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <charconv>
#include <ranges>

#include <oc/err.h>

class Require_test : public testing::Test {
protected:
    using Selected_exception = std::runtime_error;

    const bool true_condition_{ true };
    const bool false_condition_{ false };

    const std::regex error_message_pattern_regex_ = std::regex("^.+ exception \\(at line [0-9]+, .+@.+\\), assertion .+ failed: .+$");
};

TEST_F(Require_test, not_throw_exception_if_condition_is_true)
{
    EXPECT_NO_THROW(OCERR_REQUIRE(true_condition_, Selected_exception));
}

TEST_F(Require_test, throw_exception_if_condition_is_false)
{
    EXPECT_THROW(OCERR_REQUIRE(false_condition_, Selected_exception), Selected_exception);
}

struct CustomTestType {};
std::ostream& operator<<(std::ostream& os, const CustomTestType& ctt) {
    os << "CustomTestType as string";
    return os;
}

TEST_F(Require_test, throws_an_exception_with_specific_format)
{
    try {
        OCERR_REQUIRE(false_condition_, Selected_exception, (std::stringstream{} << CustomTestType{}).str());
        FAIL();
    }
    catch (const Selected_exception& ex) {
        EXPECT_TRUE(std::regex_match(ex.what(), error_message_pattern_regex_));
    }
}

namespace optional_test_dummies {
    template <typename T>
    using Optional = oc::Expected<T>;

    constexpr oc::None_option nullopt{};

    auto to_int(std::string_view sv) -> Optional<int>
    {
        int r{};
        auto [ptr, ec] { std::from_chars(sv.data(), sv.data() + sv.size(), r) };
        if (ec == std::errc()) {
            return r;
        }
        return nullopt;
    };

    auto inc(int n)
    {
        return n + 1;
    }

    auto to_string(int n)
    {
        return std::to_string(n);
    }

    auto get_null_opt() -> Optional<std::string>
    {
        return "Null";
    }
}

TEST(Expected_test, using_Expected_type_as_Optional)
{
    using namespace optional_test_dummies;

    std::vector<Optional<std::string>> input = {
        "1234", "15 foo", "bar", "42", "5000000000", " 5" };

    std::vector<std::string> results = {
        "1235", "16", "Null", "43", "Null", "Null" };

    auto to_incremented_string = [](auto&& o) {
        return o.and_then(to_int)
            .transform(inc)
            .transform(to_string)
            .or_else(get_null_opt);
    };

    std::vector<std::string> output;

    for (auto&& x : input | std::views::transform(to_incremented_string)) {
        output.push_back(*x);
    }

    EXPECT_EQ(results, output);
}

namespace expected_test_dummies {
    using namespace std::literals;

    auto to_int(std::string_view sv) -> oc::Expected<int, std::string>
    {
        int r{};
        auto [ptr, ec] { std::from_chars(sv.data(), sv.data() + sv.size(), r) };
        if (ec == std::errc()) {
            return r;
        }
        return oc::Unexpected{ "Null"s };
    };

    auto inc(int n)
    {
        return n + 1;
    }

    auto get_failure(const std::string&) -> oc::Expected<int, std::string>
    {
        return oc::Unexpected{ "conversion failed"s };
    }

    auto decorate_as_error(const std::string& s)
    {
        return "error: " + s;
    }
}

TEST(Expected_test, using_Expected_with_value_and_error)
{
    using namespace expected_test_dummies;
    using namespace oc;

    std::vector<Expected<std::string, std::string>> input = {
        "1234", "15 foo", "bar", "42", "5000000000", " 5" };

    std::vector<int> successful_results = { 1235, 16, 0, 43, 0, 0 };

    std::vector<std::string> failed_results = { "error: conversion failed", "error: conversion failed", "error: conversion failed" };

    auto to_incremented_string = [](auto&& o) {
        return o.and_then(to_int)
            .transform(inc)
            .or_else(get_failure)
            .transform_error(decorate_as_error);
    };

    std::vector<int> successful_output;
    std::vector<std::string> failed_output;

    for (auto&& x : input | std::views::transform(to_incremented_string)) {
        successful_output.push_back(x.value_or(0));
        if (!x) {
            failed_output.push_back(x.error());
        }
    }

    EXPECT_EQ(successful_results, successful_output);
    EXPECT_EQ(failed_results, failed_output);
}

