#include "mocks.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using ::testing::Return;
using ::testing::Invoke;
using ::testing::HasSubstr;

TEST(Process, UnknownCommand)
{
    auto rig = makeRig();

    auto msg = NetworkSerializer().serialize(
        pkg::Message{10, NetworkSerializer().serialize(mms::Manager{"unknown_command", ""})});

    rig.core->Process(1, "cli", msg);

    // Не должно быть ответа для неизвестной команды
    EXPECT_EQ(rig.lastWrite->size(), 0);
}

