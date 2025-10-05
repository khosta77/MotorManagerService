#include "protocol_handler.hpp"
#include <iostream>
#include <format>
#include <thread>
#include <chrono>

ProtocolHandler::CommandResult ProtocolHandler::handleVersionCommand()
{
    return {true, 0x12, "Version 1.2"};
}

ProtocolHandler::CommandResult ProtocolHandler::handleMotorCommand(
    uint8_t commandByte, 
    const std::vector<MotorData>& motorData)
{
    bool isSynchronous = (commandByte & 0xF0) == 0x80;
    size_t motorCount = commandByte & 0x0F;
    
    // Валидация команды
    if (!validateMotorCommand(commandByte, motorCount)) {
        return {false, 0x01, "Invalid motor count"};
    }
    
    // Валидация данных моторов
    for (const auto& motor : motorData) {
        if (!validateMotorParameters(motor)) {
            return {false, 0x05, "Invalid motor parameters"};
        }
    }
    
    // Симуляция обработки каждого мотора
    for (const auto& motor : motorData) {
        uint8_t result = simulateSingleMotorProcessing(motor);
        if (result != 0xFF) {
            return {false, result, getErrorDescription(result)};
        }
    }
    
    return {true, 0xFF, "Success"};
}

std::vector<ProtocolHandler::MotorData> ProtocolHandler::parseMotorData(
    const std::vector<uint8_t>& data, 
    size_t motorCount)
{
    std::vector<MotorData> motors;
    motors.reserve(motorCount);
    
    const uint32_t* ptr = reinterpret_cast<const uint32_t*>(data.data());
    
    for (size_t i = 0; i < motorCount; ++i) {
        MotorData motor;
        motor.number = ptr[i * 4 + 0];
        motor.acceleration = ptr[i * 4 + 1];
        motor.maxSpeed = ptr[i * 4 + 2];
        motor.step = static_cast<int32_t>(ptr[i * 4 + 3]);
        
        motors.push_back(motor);
    }
    
    return motors;
}

bool ProtocolHandler::validateMotorCommand(uint8_t commandByte, size_t motorCount)
{
    // Проверяем режим команды
    bool isSynchronous = (commandByte & 0xF0) == 0x80;
    bool isAsynchronous = (commandByte & 0xF0) == 0x40;
    
    if (!isSynchronous && !isAsynchronous) {
        return false;
    }
    
    // Проверяем количество моторов
    if (motorCount < 1 || motorCount > 10) {
        return false;
    }
    
    return true;
}

std::string ProtocolHandler::getErrorDescription(uint8_t errorCode)
{
    switch (errorCode) {
        case 0x01: return "Invalid motor initialization";
        case 0x02: return "Maximum speed exceeded";
        case 0x03: return "Acceleration calculation error";
        case 0x04: return "Positioning error";
        case 0x05: return "Mechanical error (motor jammed)";
        case 0x06: return "Driver overheating";
        case 0x07: return "Power error";
        case 0x08: return "Driver communication error";
        case 0x09: return "Movement limits exceeded";
        case 0x0A: return "Calibration error";
        case 0x0B: return "Emergency stop";
        case 0x0C: return "Synchronization error";
        case 0x0D: return "Command execution timeout";
        case 0x0E: return "Encoder reading error";
        case 0x0F: return "General execution error";
        default: return "Unknown error";
    }
}

uint8_t ProtocolHandler::simulateSingleMotorProcessing(const MotorData& motor)
{
    // Симуляция различных сценариев ошибок для тестирования
    
    // Проверка номера мотора
    if (motor.number < 1 || motor.number > 10) {
        return 0x01; // Ошибка инициализации мотора
    }
    
    // Проверка ускорения
    if (motor.acceleration == 0) {
        return 0x03; // Ошибка расчета ускорения
    }
    
    // Проверка максимальной скорости
    if (motor.maxSpeed == 0) {
        return 0x02; // Превышение максимальной скорости
    }
    
    // Симуляция случайных ошибок (5% вероятность)
    static bool errorSimulation = false; // Можно включить для тестирования
    if (errorSimulation) {
        static int errorCounter = 0;
        errorCounter++;
        
        // Каждый 20-й мотор будет с ошибкой для демонстрации
        if (errorCounter % 20 == 0) {
            return 0x05; // Механическая ошибка
        }
    }
    
    // Симуляция времени обработки
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    return 0xFF; // Успешное выполнение
}

bool ProtocolHandler::validateMotorParameters(const MotorData& motor)
{
    // Проверка номера мотора
    if (motor.number < 1 || motor.number > 10) {
        return false;
    }
    
    // Проверка ускорения
    if (motor.acceleration == 0) {
        return false;
    }
    
    // Проверка максимальной скорости
    if (motor.maxSpeed == 0) {
        return false;
    }
    
    return true;
}
