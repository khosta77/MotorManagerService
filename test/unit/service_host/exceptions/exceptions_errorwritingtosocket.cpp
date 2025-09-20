#include "exceptions.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

TEST(ErrorWritingToSocketTest, BasicFunctionality)
{
    const int test_socket = 99;
    ErrorWritingToSocket ex(test_socket);

    EXPECT_STREQ(ex.what(), "In \'UniversalServerMethods::writeToSock\', something error");
    EXPECT_EQ(ex.sock, test_socket);
    EXPECT_TRUE(dynamic_cast<MyException*>(&ex));
    EXPECT_TRUE(dynamic_cast<std::exception*>(&ex));
}

TEST(ErrorWritingToSocketTest, ThrowAndCatch)
{
    const int test_socket = 456;

    try
    {
        throw ErrorWritingToSocket(test_socket);
        FAIL() << "Expected ErrorWritingToSocket to be thrown";
    }
    catch (const ErrorWritingToSocket& ex)
    {
        EXPECT_EQ(ex.sock, test_socket);
        EXPECT_STREQ(ex.what(), "In \'UniversalServerMethods::writeToSock\', something error");
    }
    catch (...)
    {
        FAIL() << "Expected ErrorWritingToSocket but got different exception";
    }
}

