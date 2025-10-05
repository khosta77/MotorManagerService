#include "mocks.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using ::testing::Return;
using ::testing::Invoke;
using ::testing::HasSubstr;

TEST(Reconnect, Success)
{
    auto rig = makeRig();

    // reconnect: если уже подключено -> ошибка, значит должно быть false
    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(false));
    EXPECT_CALL(*rig.module, connect(0)).WillOnce(Return(true));

    mms::Device d{0};
    auto msg = NetworkSerializer().serialize(pkg::Message{
        3,
        NetworkSerializer().serialize(mms::Manager{"reconnect", NetworkSerializer().serialize(d)})});
    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("\"status\":0"));
}

TEST(Reconnect, BadJson)
{
    auto rig = makeRig();
    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(false));

    // message содержит некорректный json для mms::Device
    auto msg = NetworkSerializer().serialize(
        pkg::Message{4, NetworkSerializer().serialize(mms::Manager{"reconnect", "{bad json}"})});
    rig.core->Process(1, "cli", msg);
    EXPECT_THAT(*rig.lastWrite, HasSubstr("40403"));
}

TEST(Reconnect, NegativeDeviceId)
{
    auto rig = makeRig();
    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(false));

    mms::Device d{-1};
    auto msg = NetworkSerializer().serialize(pkg::Message{
        5,
        NetworkSerializer().serialize(mms::Manager{"reconnect", NetworkSerializer().serialize(d)})});
    rig.core->Process(1, "cli", msg);
    EXPECT_THAT(*rig.lastWrite, HasSubstr("40509"));
}

TEST(Reconnect, ConnectFail)
{
    auto rig = makeRig();
    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(false));
    EXPECT_CALL(*rig.module, connect(1)).WillOnce(Return(false));

    mms::Device d{1};
    auto msg = NetworkSerializer().serialize(pkg::Message{
        6,
        NetworkSerializer().serialize(mms::Manager{"reconnect", NetworkSerializer().serialize(d)})});
    rig.core->Process(1, "cli", msg);
    EXPECT_THAT(*rig.lastWrite, HasSubstr("40510"));
}

TEST(Reconnect, AlreadyConnected)
{
    auto rig = makeRig();

    // Модуль уже подключен - должна быть ошибка
    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(true));

    mms::Device d{0};
    auto msg = NetworkSerializer().serialize(pkg::Message{
        11,
        NetworkSerializer().serialize(mms::Manager{"reconnect", NetworkSerializer().serialize(d)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40512"));
}

