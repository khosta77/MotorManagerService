#include "exceptions.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

TEST(NotCorrectMessageToSendTest, BasicFunctionality)
{
    NotCorrectMessageToSend ex;

    EXPECT_STREQ(ex.what(), "Not correct message to send");
    EXPECT_TRUE(dynamic_cast<MyException*>(&ex));
    EXPECT_TRUE(dynamic_cast<std::exception*>(&ex));
}

TEST(NotCorrectMessageToSendTest, ThrowAndCatch)
{
    try
    {
        throw NotCorrectMessageToSend();
        FAIL() << "Expected NotCorrectMessageToSend to be thrown";
    }
    catch (const NotCorrectMessageToSend& ex)
    {
        EXPECT_STREQ(ex.what(), "Not correct message to send");
    }
    catch (...)
    {
        FAIL() << "Expected NotCorrectMessageToSend but got different exception";
    }
}

