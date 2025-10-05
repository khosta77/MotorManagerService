#include "exceptions.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

TEST(MyExceptionTest, ConstructorAndWhat)
{
    const std::string error_msg = "Test error message";
    MyException ex(error_msg);

    EXPECT_STREQ(ex.what(), error_msg.c_str());
    EXPECT_EQ(std::string(ex.what()), error_msg);
}

TEST(MyExceptionTest, ThrowsCorrectly)
{
    const std::string error_msg = "Exception thrown";

    EXPECT_THROW({ throw MyException(error_msg); }, MyException);

    try
    {
        throw MyException(error_msg);
        FAIL() << "Expected MyException to be thrown";
    }
    catch (const MyException& ex)
    {
        EXPECT_STREQ(ex.what(), error_msg.c_str());
    }
    catch (...)
    {
        FAIL() << "Expected MyException but got different exception";
    }
}

TEST(MyExceptionTest, NoexceptSpecification)
{
    MyException ex("Test");
    EXPECT_TRUE(noexcept(ex.what()));
}

TEST(MyExceptionTest, EmptyMessage)
{
    MyException ex("");
    EXPECT_STREQ(ex.what(), "");
}

TEST(MyExceptionTest, CopyConstructor)
{
    MyException ex1("Original");
    MyException ex2 = ex1;

    EXPECT_STREQ(ex1.what(), ex2.what());
}

TEST(MyExceptionTest, MoveConstructor)
{
    MyException ex1("Original");
    MyException ex2 = std::move(ex1);

    EXPECT_STREQ(ex2.what(), "Original");
}

