#include "exceptions.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

TEST(ErrorReadingFromSocketTest, BasicFunctionality)
{
    const int test_socket = 42;
    ErrorReadingFromSocket ex(test_socket);

    EXPECT_STREQ(ex.what(), "In \'UniversalServerMethods::readFromSock\', something error");
    EXPECT_EQ(ex.sock, test_socket);
    EXPECT_TRUE(dynamic_cast<MyException*>(&ex));
    EXPECT_TRUE(dynamic_cast<std::exception*>(&ex));
}

TEST(ErrorReadingFromSocketTest, ThrowAndCatch)
{
    const int test_socket = 123;

    try
    {
        throw ErrorReadingFromSocket(test_socket);
        FAIL() << "Expected ErrorReadingFromSocket to be thrown";
    }
    catch (const ErrorReadingFromSocket& ex)
    {
        EXPECT_EQ(ex.sock, test_socket);
        EXPECT_STREQ(ex.what(), "In \'UniversalServerMethods::readFromSock\', something error");
    }
    catch (...)
    {
        FAIL() << "Expected ErrorReadingFromSocket but got different exception";
    }
}

