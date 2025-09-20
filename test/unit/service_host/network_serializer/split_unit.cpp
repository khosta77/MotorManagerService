#include "network_serializer.hpp"

#include <gtest/gtest.h>

//// split

TEST(splitTest, EmptyString)
{
    NetworkSerializer serverMethods;
    const std::string input = "";
    std::vector<std::string> expected = {};
    auto result = serverMethods.split(input);
    EXPECT_EQ(result, expected);
}

TEST(splitTest, NoDelimiters)
{
    NetworkSerializer serverMethods;
    const std::string input = "single message";
    std::vector<std::string> expected = {"single message"};
    auto result = serverMethods.split(input);
    EXPECT_EQ(result, expected);
}

TEST(splitTest, SingleDelimiterAtEnd)
{
    NetworkSerializer serverMethods;
    const std::string input = "message\n\n";
    std::vector<std::string> expected = {"message"};
    auto result = serverMethods.split(input);
    EXPECT_EQ(result, expected);
}

TEST(splitTest, SingleDelimiterInMiddle)
{
    NetworkSerializer serverMethods;
    const std::string input = "first\n\nsecond";
    std::vector<std::string> expected = {"first", "second"};
    auto result = serverMethods.split(input);
    EXPECT_EQ(result, expected);
}

TEST(splitTest, MultipleDelimiters)
{
    NetworkSerializer serverMethods;
    const std::string input = "first\n\nsecond\n\nthird";
    std::vector<std::string> expected = {"first", "second", "third"};
    auto result = serverMethods.split(input);
    EXPECT_EQ(result, expected);
}

TEST(splitTest, ConsecutiveDelimiters)
{
    NetworkSerializer serverMethods;
    const std::string input = "first\n\n\n\nsecond";
    std::vector<std::string> expected = {"first", "second"};
    auto result = serverMethods.split(input);
    EXPECT_EQ(result, expected);
}

TEST(splitTest, DelimiterAtStart)
{
    NetworkSerializer serverMethods;
    const std::string input = "\n\nmessage";
    std::vector<std::string> expected = {"message"};
    auto result = serverMethods.split(input);
    EXPECT_EQ(result, expected);
}

TEST(splitTest, OnlyDelimiters)
{
    NetworkSerializer serverMethods;
    const std::string input = "\n\n\n\n";
    std::vector<std::string> expected = {};
    auto result = serverMethods.split(input);
    EXPECT_EQ(result, expected);
}

TEST(splitTest, OneDelimiters)
{
    NetworkSerializer serverMethods;
    const std::string input = "\n\n\n";
    std::vector<std::string> expected = {"\n"};
    auto result = serverMethods.split(input);
    EXPECT_EQ(result, expected);
}

