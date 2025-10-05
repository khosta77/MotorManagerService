#include "mocks.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using ::testing::Return;
using ::testing::Invoke;
using ::testing::HasSubstr;

TEST(Version, Success)
{
    auto rig = makeRig();

    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(true));
    EXPECT_CALL(*rig.module, writeData(_));
    EXPECT_CALL(*rig.module, readData(_)).WillOnce(Invoke([](std::vector<uchar>& data) {
        // version: 1.3 -> 0001 0011 (0x13)
        if (data.empty())
            data.push_back(0);
        data[0] = 0x13;
    }));

    rig.core->Process(
        1,
        "cli",
        NetworkSerializer().serialize(
            pkg::Message{1, NetworkSerializer().serialize(mms::Manager{"version", ""})}));

    EXPECT_THAT(*rig.lastWrite, HasSubstr("\"what\":\"mms::Version\""));
    EXPECT_THAT(*rig.lastWrite, HasSubstr("Squid"));
}

TEST(Version, MessageNotEmpty)
{
    auto rig = makeRig();

    rig.core->Process(
        1,
        "cli",
        NetworkSerializer().serialize(
            pkg::Message{1, NetworkSerializer().serialize(mms::Manager{"version", "not-empty"})}));

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40506"));
}

TEST(Version, NotConnected)
{
    auto rig = makeRig();
    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(false));

    rig.core->Process(
        1,
        "cli",
        NetworkSerializer().serialize(
            pkg::Message{2, NetworkSerializer().serialize(mms::Manager{"version", ""})}));
    EXPECT_THAT(*rig.lastWrite, HasSubstr("40507"));
}

