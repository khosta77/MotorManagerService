#include "mocks.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using ::testing::Return;
using ::testing::Invoke;
using ::testing::HasSubstr;

TEST(Moving, NotConnected)
{
    auto rig = makeRig();

    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(false));

    mms::MotorsSettings settings;
    settings.mode = "synchronous";
    mms::Motor motor;
    motor.number = 1;
    motor.acceleration = 100;
    motor.maxSpeed = 1000;
    settings.motors.push_back(motor);

    auto msg = NetworkSerializer().serialize(pkg::Message{
        12,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40507"));
}

TEST(Moving, InvalidMotorsSettingsJson)
{
    auto rig = makeRig();

    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(true));

    // Некорректный JSON для MotorsSettings
    auto msg = NetworkSerializer().serialize(
        pkg::Message{13, NetworkSerializer().serialize(mms::Manager{"moving", "{invalid json}"})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40402"));
}

TEST(Moving, InvalidMode)
{
    auto rig = makeRig();

    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(true));

    mms::MotorsSettings settings;
    settings.mode = "invalid_mode"; // Невалидный режим
    mms::Motor motor;
    motor.number = 1;
    motor.acceleration = 100;
    motor.maxSpeed = 1000;
    settings.motors.push_back(motor);

    auto msg = NetworkSerializer().serialize(pkg::Message{
        14,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40501"));
}

TEST(Moving, TooManyMotors)
{
    auto rig = makeRig();

    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(true));

    mms::MotorsSettings settings;
    settings.mode = "synchronous";

    // Создаем больше 10 моторов
    for (int i = 1; i <= 15; ++i)
    {
        mms::Motor motor;
        motor.number = i;
        motor.acceleration = 100;
        motor.maxSpeed = 1000;
        settings.motors.push_back(motor);
    }

    auto msg = NetworkSerializer().serialize(pkg::Message{
        15,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40502"));
}

TEST(Moving, InvalidMotorNumber)
{
    auto rig = makeRig();

    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(true));

    mms::MotorsSettings settings;
    settings.mode = "synchronous";

    mms::Motor motor;
    motor.number = 15; // Невалидный номер (должен быть 1-10)
    motor.acceleration = 100;
    motor.maxSpeed = 1000;
    settings.motors.push_back(motor);

    auto msg = NetworkSerializer().serialize(pkg::Message{
        16,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40503"));
}

TEST(Moving, ZeroAcceleration)
{
    auto rig = makeRig();

    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(true));

    mms::MotorsSettings settings;
    settings.mode = "synchronous";

    mms::Motor motor;
    motor.number = 1;
    motor.acceleration = 0; // Нулевое ускорение
    motor.maxSpeed = 1000;
    settings.motors.push_back(motor);

    auto msg = NetworkSerializer().serialize(pkg::Message{
        17,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40504"));
}

TEST(Moving, ZeroMaxSpeed)
{
    auto rig = makeRig();

    EXPECT_CALL(*rig.module, isConnected()).WillOnce(Return(true));

    mms::MotorsSettings settings;
    settings.mode = "synchronous";

    mms::Motor motor;
    motor.number = 1;
    motor.acceleration = 100;
    motor.maxSpeed = 0; // Нулевая максимальная скорость
    settings.motors.push_back(motor);

    auto msg = NetworkSerializer().serialize(pkg::Message{
        18,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40505"));
}

