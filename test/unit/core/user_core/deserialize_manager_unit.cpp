#include "mocks.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using ::testing::Return;
using ::testing::Invoke;
using ::testing::HasSubstr;

TEST(DeserializeMessage, InvalidJson)
{
    auto rig = makeRig();

    // Отправляем некорректный JSON
    rig.core->Process(1, "cli", "invalid json data");

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40401"));
}

TEST(DeserializeManager, InvalidJson)
{
    auto rig = makeRig();

    // Создаем валидное pkg::Message но с некорректным JSON для Manager
    pkg::Message msg;
    msg.id = 1;
    msg.text = "invalid manager json";

    rig.core->Process(1, "cli", NetworkSerializer().serialize(msg));

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40401"));
}

