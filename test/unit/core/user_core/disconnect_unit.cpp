#include "mocks.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using ::testing::Return;
using ::testing::Invoke;
using ::testing::HasSubstr;

TEST(Disconnect, NotConnected)
{
    auto rig = makeRig();
    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(false));

    auto msg = NetworkSerializer().serialize(
        pkg::Message{7, NetworkSerializer().serialize(mms::Manager{"disconnect", ""})});
    rig.core->Process(1, "cli", msg);
    EXPECT_THAT(*rig.lastWrite, HasSubstr("40507"));
}

TEST(Disconnect, Success)
{
    auto rig = makeRig();
    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(*rig.module, disconnect()).Times(1);

    auto msg = NetworkSerializer().serialize(
        pkg::Message{8, NetworkSerializer().serialize(mms::Manager{"disconnect", ""})});
    rig.core->Process(1, "cli", msg);
    EXPECT_THAT(*rig.lastWrite, HasSubstr("\"status\":0"));
}

TEST(Disconnect, NonEmptyMessage)
{
    auto rig = makeRig();

    // Сообщение не должно быть пустым для disconnect
    auto msg = NetworkSerializer().serialize(
        pkg::Message{19, NetworkSerializer().serialize(mms::Manager{"disconnect", "non-empty"})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40506"));
}

