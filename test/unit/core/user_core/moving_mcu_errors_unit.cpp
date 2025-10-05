#include "mocks.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using ::testing::Return;
using ::testing::Invoke;
using ::testing::HasSubstr;
using ::testing::Sequence;

// Тесты для различных кодов ошибок MCU согласно документации

TEST(MovingMCUErrors, ReadinessErrorCodes)
{
    struct MCUErrorTest {
        uint8_t errorCode;
        std::string description;
    };
    
    std::vector<MCUErrorTest> readinessErrors = {
        {0x01, "Некорректное количество моторов"},
        {0x02, "Некорректный режим работы"},
        {0x03, "MCU занят другой операцией"},
        {0x04, "Недостаточно памяти для обработки"},
        {0x05, "Ошибка инициализации моторов"},
        {0x06, "Ошибка конфигурации портов"},
        {0x07, "Ошибка таймеров"},
        {0x08, "Ошибка DMA"},
        {0x09, "Ошибка прерываний"},
        {0x0A, "Общая ошибка системы"}
    };
    
    for (const auto& error : readinessErrors) {
        auto rig = makeRig();
        
        Sequence seq;
        EXPECT_CALL(*rig.module, isConnected()).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq);
        
        EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
            .WillOnce(Invoke([errorCode = error.errorCode](std::vector<uchar>& data) {
                if (data.empty()) data.resize(1);
                data[0] = errorCode;
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
            200 + error.errorCode,
            NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});
        
        rig.core->Process(1, "cli", msg);
        
        EXPECT_THAT(*rig.lastWrite, HasSubstr("40512")) << "Failed for error code 0x" << std::hex << error.errorCode;
        EXPECT_THAT(*rig.lastWrite, HasSubstr("MCU readiness error")) << "Failed for error code 0x" << std::hex << error.errorCode;
    }
}

TEST(MovingMCUErrors, ExecutionErrorCodes)
{
    struct MCUExecutionErrorTest {
        uint8_t errorCode;
        std::string description;
    };
    
    std::vector<MCUExecutionErrorTest> executionErrors = {
        {0x01, "Ошибка инициализации мотора"},
        {0x02, "Превышение максимальной скорости"},
        {0x03, "Ошибка расчета ускорения"},
        {0x04, "Ошибка позиционирования"},
        {0x05, "Механическая ошибка (заклинивание)"},
        {0x06, "Перегрев драйвера"},
        {0x07, "Ошибка питания"},
        {0x08, "Ошибка связи с драйвером"},
        {0x09, "Превышение лимитов движения"},
        {0x0A, "Ошибка калибровки"},
        {0x0B, "Аварийная остановка"},
        {0x0C, "Ошибка синхронизации (для синхронного режима)"},
        {0x0D, "Таймаут выполнения команды"},
        {0x0E, "Ошибка чтения энкодера"},
        {0x0F, "Общая ошибка выполнения"}
    };
    
    for (const auto& error : executionErrors) {
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
        
        EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
            .WillOnce(Invoke([errorCode = error.errorCode](std::vector<uchar>& data) {
                if (data.empty()) data.resize(1);
                data[0] = errorCode;
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
            300 + error.errorCode,
            NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});
        
        rig.core->Process(1, "cli", msg);
        
        EXPECT_THAT(*rig.lastWrite, HasSubstr("40513")) << "Failed for error code 0x" << std::hex << error.errorCode;
        EXPECT_THAT(*rig.lastWrite, HasSubstr("MCU execution error")) << "Failed for error code 0x" << std::hex << error.errorCode;
    }
}

TEST(MovingProtocol, CommandByteGeneration)
{
    // Тестируем правильность генерации командных байтов для разных режимов и количества моторов
    
    struct CommandTest {
        std::string mode;
        size_t motorCount;
        uint8_t expectedCommand;
    };
    
    std::vector<CommandTest> commandTests = {
        {"synchronous", 1, 0x81},   // 0x80 | 1
        {"synchronous", 2, 0x82},  // 0x80 | 2
        {"synchronous", 10, 0x8A}, // 0x80 | 10
        {"asynchronous", 1, 0x41},  // 0x40 | 1
        {"asynchronous", 2, 0x42}, // 0x40 | 2
        {"asynchronous", 10, 0x4A} // 0x40 | 10
    };
    
    for (const auto& test : commandTests) {
        auto rig = makeRig();
        
        Sequence seq;
        EXPECT_CALL(*rig.module, isConnected()).InSequence(seq).WillOnce(Return(true));
        
        EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq)
            .WillOnce(Invoke([expected = test.expectedCommand, mode = test.mode, count = test.motorCount](const std::vector<uchar>& data) {
                EXPECT_EQ(data.size(), 1);
                EXPECT_EQ(data[0], expected) << "Mode: " << mode << ", Count: " << count;
            }));
        
        EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
            .WillOnce(Invoke([](std::vector<uchar>& data) {
                if (data.empty()) data.resize(1);
                data[0] = 0x00; // Готовность OK
            }));
        
        EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq);
        EXPECT_CALL(*rig.module, checkRXChannel()).InSequence(seq).WillOnce(Return(1));
        EXPECT_CALL(*rig.module, readData(testing::_)).InSequence(seq)
            .WillOnce(Invoke([](std::vector<uchar>& data) {
                if (data.empty()) data.resize(1);
                data[0] = 0xFF; // Успешное выполнение
            }));
        
        mms::MotorsSettings settings;
        settings.mode = test.mode;
        
        // Создаем нужное количество моторов
        for (size_t i = 1; i <= test.motorCount; ++i) {
            mms::Motor motor;
            motor.number = static_cast<int>(i);
            motor.acceleration = 2000;
            motor.maxSpeed = 5000;
            motor.step = 100;
            settings.motors.push_back(motor);
        }
        
        auto msg = NetworkSerializer().serialize(pkg::Message{
            static_cast<int>(400 + test.motorCount),
            NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});
        
        rig.core->Process(1, "cli", msg);
        
        EXPECT_THAT(*rig.lastWrite, HasSubstr("\"status\":0")) 
            << "Mode: " << test.mode << ", Count: " << test.motorCount;
    }
}

TEST(MovingProtocol, DataIntegrityCheck)
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
    
    // Проверяем целостность данных при передаче больших значений
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](const std::vector<uchar>& data) {
            EXPECT_EQ(data.size(), 16);
            
            const uint32_t* ptr = reinterpret_cast<const uint32_t*>(data.data());
            
            // Проверяем передачу максимальных значений
            EXPECT_EQ(ptr[0], 10);           // number (максимальный номер мотора)
            EXPECT_EQ(ptr[1], 0xFFFFFFFF);   // acceleration (максимальное значение)
            EXPECT_EQ(ptr[2], 0xFFFFFFFF);   // maxSpeed (максимальное значение)
            EXPECT_EQ(ptr[3], 0x7FFFFFFF);   // step (максимальное положительное значение)
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
    motor.number = 10;                    // Максимальный номер мотора
    motor.acceleration = 0xFFFFFFFF;      // Максимальное ускорение
    motor.maxSpeed = 0xFFFFFFFF;          // Максимальная скорость
    motor.step = 0x7FFFFFFF;              // Максимальное положительное значение шагов
    settings.motors.push_back(motor);
    
    auto msg = NetworkSerializer().serialize(pkg::Message{
        500,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});
    
    rig.core->Process(1, "cli", msg);
    
    EXPECT_THAT(*rig.lastWrite, HasSubstr("\"status\":0"));
}

TEST(MovingProtocol, EdgeCaseValues)
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
    
    // Проверяем обработку граничных значений
    EXPECT_CALL(*rig.module, writeData(testing::_)).InSequence(seq)
        .WillOnce(Invoke([](const std::vector<uchar>& data) {
            EXPECT_EQ(data.size(), 16);
            
            const uint32_t* ptr = reinterpret_cast<const uint32_t*>(data.data());
            
            EXPECT_EQ(ptr[0], 1);           // number (минимальный номер)
            EXPECT_EQ(ptr[1], 1);           // acceleration (минимальное значение > 0)
            EXPECT_EQ(ptr[2], 1);           // maxSpeed (минимальное значение > 0)
            EXPECT_EQ(ptr[3], 0x80000000); // step (минимальное отрицательное значение)
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
    motor.number = 1;                      // Минимальный номер мотора
    motor.acceleration = 1;                // Минимальное ускорение > 0
    motor.maxSpeed = 1;                    // Минимальная скорость > 0
    motor.step = static_cast<int32_t>(0x80000000); // Минимальное отрицательное значение
    settings.motors.push_back(motor);
    
    auto msg = NetworkSerializer().serialize(pkg::Message{
        501,
        NetworkSerializer().serialize(mms::Manager{"moving", NetworkSerializer().serialize(settings)})});
    
    rig.core->Process(1, "cli", msg);
    
    EXPECT_THAT(*rig.lastWrite, HasSubstr("\"status\":0"));
}
