#include "mocks.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using ::testing::Return;
using ::testing::Invoke;
using ::testing::HasSubstr;
using ::testing::Sequence;
using ::testing::InSequence;

// Тесты для протокола взаимодействия с MCU в команде moving()

TEST(MovingProtocol, SynchronousSuccess)
{
    auto rig = makeRig();

    // Настройка последовательности вызовов для успешного выполнения
    Sequence seq;
    EXPECT_CALL(*rig.module, isConnected()).InSequence(seq).WillOnce(Return(true));
    
    // Первая запись - команда режима и количества моторов (0x81 для синхронного режима с 1 мотором)
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](const std::vector<uchar>& data) {
            EXPECT_EQ(data.size(), 1);
            EXPECT_EQ(data[0], 0x81); // 0x80 | 1 = синхронный режим, 1 мотор
        }));
    
    // Чтение подтверждения готовности MCU (0x00 = OK)
    EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](std::vector<uchar>& data) {
            if (data.empty()) data.resize(1);
            data[0] = 0x00; // Готовность OK
        }));
    
    // Вторая запись - параметры моторов (16 байт для 1 мотора)
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](const std::vector<uchar>& data) {
            EXPECT_EQ(data.size(), 16); // 4 параметра × 4 байта
            // Проверяем первые 4 байта (number = 1)
            uint32_t number = *reinterpret_cast<const uint32_t*>(data.data());
            EXPECT_EQ(number, 1);
        }));
    
    // Проверка RX канала для таймаута
    EXPECT_CALL(*rig.module, checkRXChannel()).InSequence(seq).WillOnce(Return(1));
    
    // Чтение результата выполнения (0xFF = успех)
    EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](std::vector<uchar>& data) {
            if (data.empty()) data.resize(1);
            data[0] = 0xFF; // Успешное выполнение
        }));

    mms::MotorsSettings settings;
    settings.mode = "synchronous";
    mms::Motor motor;
    motor.number = 1;
    motor.acceleration = 2000;
    motor.maxSpeed = 5000;
    motor.step = 100;
    settings.motors.push_back(motor);

    auto msg = NetworkSerializer().serialize(pkg::Message{
        100,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("\"status\":0"));
    EXPECT_THAT(*rig.lastWrite, HasSubstr("\"what\":\"\""));
    EXPECT_THAT(*rig.lastWrite, HasSubstr("\"subMessage\":\"\""));
}

TEST(MovingProtocol, AsynchronousSuccess)
{
    auto rig = makeRig();

    Sequence seq;
    EXPECT_CALL(*rig.module, isConnected()).InSequence(seq).WillOnce(Return(true));
    
    // Команда для асинхронного режима с 2 моторами (0x42)
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](const std::vector<uchar>& data) {
            EXPECT_EQ(data.size(), 1);
            EXPECT_EQ(data[0], 0x42); // 0x40 | 2 = асинхронный режим, 2 мотора
        }));
    
    EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](std::vector<uchar>& data) {
            if (data.empty()) data.resize(1);
            data[0] = 0x00; // Готовность OK
        }));
    
    // 32 байта для 2 моторов (2 × 16 байт)
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](const std::vector<uchar>& data) {
            EXPECT_EQ(data.size(), 32); // 2 мотора × 16 байт
        }));
    
    EXPECT_CALL(*rig.module, checkRXChannel()).InSequence(seq).WillOnce(Return(1));
    
    EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](std::vector<uchar>& data) {
            if (data.empty()) data.resize(1);
            data[0] = 0xFF; // Успешное выполнение
        }));

    mms::MotorsSettings settings;
    settings.mode = "asynchronous";
    
    // Два мотора
    mms::Motor motor1;
    motor1.number = 1;
    motor1.acceleration = 2000;
    motor1.maxSpeed = 5000;
    motor1.step = 100;
    settings.motors.push_back(motor1);
    
    mms::Motor motor2;
    motor2.number = 2;
    motor2.acceleration = 1500;
    motor2.maxSpeed = 4500;
    motor2.step = -50;
    settings.motors.push_back(motor2);

    auto msg = NetworkSerializer().serialize(pkg::Message{
        101,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("\"status\":0"));
}

TEST(MovingProtocol, MCUReadinessError)
{
    auto rig = makeRig();

    Sequence seq;
    EXPECT_CALL(*rig.module, isConnected()).InSequence(seq).WillOnce(Return(true));
    
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq);
    
    // MCU возвращает ошибку готовности (0x03 = MCU busy)
    EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](std::vector<uchar>& data) {
            if (data.empty()) data.resize(1);
            data[0] = 0x03; // MCU busy with another operation
        }));

    mms::MotorsSettings settings;
    settings.mode = "synchronous";
    mms::Motor motor;
    motor.number = 1;
    motor.acceleration = 2000;
    motor.maxSpeed = 5000;
    motor.step = 100;
    settings.motors.push_back(motor);

    auto msg = NetworkSerializer().serialize(pkg::Message{
        102,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40512"));
    EXPECT_THAT(*rig.lastWrite, HasSubstr("MCU readiness error"));
    EXPECT_THAT(*rig.lastWrite, HasSubstr("0x03"));
}

TEST(MovingProtocol, MCUExecutionError)
{
    auto rig = makeRig();

    Sequence seq;
    EXPECT_CALL(*rig.module, isConnected()).InSequence(seq).WillOnce(Return(true));
    
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq); // команда
    EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](std::vector<uchar>& data) {
            if (data.empty()) data.resize(1);
            data[0] = 0x00; // Готовность OK
        }));
    
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq); // параметры моторов
    EXPECT_CALL(*rig.module, checkRXChannel()).InSequence(seq).WillOnce(Return(1));
    
    // MCU возвращает ошибку выполнения (0x05 = механическая ошибка)
    EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](std::vector<uchar>& data) {
            if (data.empty()) data.resize(1);
            data[0] = 0x05; // Mechanical error - motor jammed
        }));

    mms::MotorsSettings settings;
    settings.mode = "synchronous";
    mms::Motor motor;
    motor.number = 1;
    motor.acceleration = 2000;
    motor.maxSpeed = 5000;
    motor.step = 100;
    settings.motors.push_back(motor);

    auto msg = NetworkSerializer().serialize(pkg::Message{
        103,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40513"));
    EXPECT_THAT(*rig.lastWrite, HasSubstr("MCU execution error"));
    EXPECT_THAT(*rig.lastWrite, HasSubstr("0x05"));
}

TEST(MovingProtocol, TimeoutError)
{
    auto rig = makeRig();

    Sequence seq;
    EXPECT_CALL(*rig.module, isConnected()).InSequence(seq).WillOnce(Return(true));
    
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq); // команда
    EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](std::vector<uchar>& data) {
            if (data.empty()) data.resize(1);
            data[0] = 0x00; // Готовность OK
        }));
    
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq); // параметры моторов
    
    // Симулируем таймаут - checkRXChannel всегда возвращает 0
    EXPECT_CALL(*rig.module, checkRXChannel()).InSequence(seq).WillRepeatedly(Return(0));

    mms::MotorsSettings settings;
    settings.mode = "synchronous";
    mms::Motor motor;
    motor.number = 1;
    motor.acceleration = 2000;
    motor.maxSpeed = 5000;
    motor.step = 100;
    settings.motors.push_back(motor);

    auto msg = NetworkSerializer().serialize(pkg::Message{
        104,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("40511"));
    EXPECT_THAT(*rig.lastWrite, HasSubstr("Timeout waiting for MCU response"));
}

TEST(MovingProtocol, MotorDataFormat)
{
    auto rig = makeRig();

    Sequence seq;
    EXPECT_CALL(*rig.module, isConnected()).InSequence(seq).WillOnce(Return(true));
    
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq); // команда
    EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](std::vector<uchar>& data) {
            if (data.empty()) data.resize(1);
            data[0] = 0x00; // Готовность OK
        }));
    
    // Проверяем правильность формата данных моторов
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](const std::vector<uchar>& data) {
            EXPECT_EQ(data.size(), 16); // 1 мотор × 16 байт
            
            // Проверяем данные моторов в правильном порядке
            const uint32_t* ptr = reinterpret_cast<const uint32_t*>(data.data());
            
            EXPECT_EQ(ptr[0], 1);        // number
            EXPECT_EQ(ptr[1], 2000);     // acceleration
            EXPECT_EQ(ptr[2], 5000);     // maxSpeed
            EXPECT_EQ(ptr[3], 100);      // step
        }));
    
    EXPECT_CALL(*rig.module, checkRXChannel()).InSequence(seq).WillOnce(Return(1));
    EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](std::vector<uchar>& data) {
            if (data.empty()) data.resize(1);
            data[0] = 0xFF; // Успешное выполнение
        }));

    mms::MotorsSettings settings;
    settings.mode = "synchronous";
    mms::Motor motor;
    motor.number = 1;
    motor.acceleration = 2000;
    motor.maxSpeed = 5000;
    motor.step = 100;
    settings.motors.push_back(motor);

    auto msg = NetworkSerializer().serialize(pkg::Message{
        105,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("\"status\":0"));
}

TEST(MovingProtocol, NegativeStepValue)
{
    auto rig = makeRig();

    Sequence seq;
    EXPECT_CALL(*rig.module, isConnected()).InSequence(seq).WillOnce(Return(true));
    
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq); // команда
    EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](std::vector<uchar>& data) {
            if (data.empty()) data.resize(1);
            data[0] = 0x00; // Готовность OK
        }));
    
    // Проверяем обработку отрицательного значения step
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](const std::vector<uchar>& data) {
            EXPECT_EQ(data.size(), 16);
            
            const uint32_t* ptr = reinterpret_cast<const uint32_t*>(data.data());
            EXPECT_EQ(ptr[0], 1);        // number
            EXPECT_EQ(ptr[1], 1500);     // acceleration
            EXPECT_EQ(ptr[2], 4500);     // maxSpeed
            EXPECT_EQ(ptr[3], static_cast<uint32_t>(-50)); // step (отрицательное значение)
        }));
    
    EXPECT_CALL(*rig.module, checkRXChannel()).InSequence(seq).WillOnce(Return(1));
    EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](std::vector<uchar>& data) {
            if (data.empty()) data.resize(1);
            data[0] = 0xFF; // Успешное выполнение
        }));

    mms::MotorsSettings settings;
    settings.mode = "asynchronous";
    mms::Motor motor;
    motor.number = 1;
    motor.acceleration = 1500;
    motor.maxSpeed = 4500;
    motor.step = -50; // Отрицательное значение шагов
    settings.motors.push_back(motor);

    auto msg = NetworkSerializer().serialize(pkg::Message{
        106,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("\"status\":0"));
}

TEST(MovingProtocol, MultipleMotorsDataFormat)
{
    auto rig = makeRig();

    Sequence seq;
    EXPECT_CALL(*rig.module, isConnected()).InSequence(seq).WillOnce(Return(true));
    
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq); // команда (0x83 для 3 моторов)
    EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](std::vector<uchar>& data) {
            if (data.empty()) data.resize(1);
            data[0] = 0x00; // Готовность OK
        }));
    
    // Проверяем данные для 3 моторов (48 байт)
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](const std::vector<uchar>& data) {
            EXPECT_EQ(data.size(), 48); // 3 мотора × 16 байт
            
            const uint32_t* ptr = reinterpret_cast<const uint32_t*>(data.data());
            
            // Мотор 1
            EXPECT_EQ(ptr[0], 1);        // number
            EXPECT_EQ(ptr[1], 2000);     // acceleration
            EXPECT_EQ(ptr[2], 5000);     // maxSpeed
            EXPECT_EQ(ptr[3], 100);      // step
            
            // Мотор 2
            EXPECT_EQ(ptr[4], 2);        // number
            EXPECT_EQ(ptr[5], 1500);     // acceleration
            EXPECT_EQ(ptr[6], 4500);     // maxSpeed
            EXPECT_EQ(ptr[7], -50);      // step
            
            // Мотор 3
            EXPECT_EQ(ptr[8], 3);        // number
            EXPECT_EQ(ptr[9], 3000);     // acceleration
            EXPECT_EQ(ptr[10], 6000);    // maxSpeed
            EXPECT_EQ(ptr[11], 200);     // step
        }));
    
    EXPECT_CALL(*rig.module, checkRXChannel()).InSequence(seq).WillOnce(Return(1));
    EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](std::vector<uchar>& data) {
            if (data.empty()) data.resize(1);
            data[0] = 0xFF; // Успешное выполнение
        }));

    mms::MotorsSettings settings;
    settings.mode = "synchronous";
    
    // Три мотора
    mms::Motor motor1;
    motor1.number = 1;
    motor1.acceleration = 2000;
    motor1.maxSpeed = 5000;
    motor1.step = 100;
    settings.motors.push_back(motor1);
    
    mms::Motor motor2;
    motor2.number = 2;
    motor2.acceleration = 1500;
    motor2.maxSpeed = 4500;
    motor2.step = -50;
    settings.motors.push_back(motor2);
    
    mms::Motor motor3;
    motor3.number = 3;
    motor3.acceleration = 3000;
    motor3.maxSpeed = 6000;
    motor3.step = 200;
    settings.motors.push_back(motor3);

    auto msg = NetworkSerializer().serialize(pkg::Message{
        107,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});

    rig.core->Process(1, "cli", msg);

    EXPECT_THAT(*rig.lastWrite, HasSubstr("\"status\":0"));
}
