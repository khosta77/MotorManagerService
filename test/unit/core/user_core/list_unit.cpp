#include "mocks.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using ::testing::Return;
using ::testing::Invoke;
using ::testing::HasSubstr;

TEST(ListConnect_Group, Success)
{
    auto rig = makeRig();
    EXPECT_CALL(*rig.module, listComs()).WillOnce(Return(std::vector<std::string>{"COM0", "COM1"}));

    auto msg = NetworkSerializer().serialize(
        pkg::Message{9, NetworkSerializer().serialize(mms::Manager{"listconnect", ""})});
    rig.core->Process(1, "cli", msg);
    EXPECT_THAT(*rig.lastWrite, HasSubstr("mms::ListConnect"));
    EXPECT_THAT(*rig.lastWrite, HasSubstr("COM0"));
    EXPECT_THAT(*rig.lastWrite, HasSubstr("COM1"));
}

#if 0 // TODO: #003
TEST(ListConnect, NonEmptyMessage)
{
    auto rig = makeRig();

    EXPECT_CALL(*rig.module, listComs()).WillOnce(Return(std::vector<std::string>{"COM0"}));

    // Сообщение не должно быть пустым для listconnect
    auto msg = NetworkSerializer().serialize(
        pkg::Message{20, NetworkSerializer().serialize(mms::Manager{"listconnect", "non-empty"})});

    rig.core->Process(1, "cli", msg);

    // Должна быть ошибка из-за непустого сообщения
    EXPECT_THAT(*rig.lastWrite, HasSubstr("40506"));
}
#endif
