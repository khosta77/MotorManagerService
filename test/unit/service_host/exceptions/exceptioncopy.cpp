#include "exceptions.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

TEST(ExceptionCopyTest, CopyBehavior)
{
    const int test_socket = 789;
    ErrorReadingFromSocket ex1(test_socket);
    ErrorReadingFromSocket ex2 = ex1;

    EXPECT_EQ(ex2.sock, test_socket);
    EXPECT_STREQ(ex2.what(), ex1.what());
}

