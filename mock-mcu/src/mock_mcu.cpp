#include "mock_mcu.hpp"
#include "protocol_handler.hpp"
#include "ft232rl.hpp"
#include "exceptions.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <format>
#include <iomanip>

MockMCU::MockMCU() 
    : m_running(false)
    , m_initialized(false)
{
    m_statistics = {};
}

MockMCU::~MockMCU()
{
    stop();
}

bool MockMCU::initialize(int deviceId)
{
    try {
        m_module = std::make_unique<FT232RL>();
        
        if (!m_module->connect(deviceId)) {
            std::cerr << "Failed to connect to FT232RL device " << deviceId << std::endl;
            return false;
        }
        
        // Настройка параметров связи
        m_module->setBaudRate(9600);
        m_module->setUSBParameters(256, 256);
        m_module->setCharacteristics(FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
        
        m_initialized = true;
        logEvent(std::format("MockMCU initialized on device {}", deviceId));
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error initializing MockMCU: " << e.what() << std::endl;
        return false;
    }
}

void MockMCU::start()
{
    if (!m_initialized) {
        std::cerr << "MockMCU not initialized" << std::endl;
        return;
    }
    
    if (m_running) {
        std::cerr << "MockMCU already running" << std::endl;
        return;
    }
    
    m_running = true;
    m_workerThread = std::thread(&MockMCU::workerLoop, this);
    
    logEvent("MockMCU started");
}

void MockMCU::stop()
{
    if (!m_running) {
        return;
    }
    
    m_running = false;
    
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
    
    if (m_module) {
        m_module->disconnect();
    }
    
    logEvent("MockMCU stopped");
}

bool MockMCU::isRunning() const
{
    return m_running;
}

MockMCU::Statistics MockMCU::getStatistics() const
{
    std::lock_guard<std::mutex> lock(m_statsMutex);
    return m_statistics;
}

void MockMCU::workerLoop()
{
    logEvent("Worker thread started");
    
    while (m_running) {
        try {
            // Проверяем наличие данных для чтения
            size_t availableBytes = m_module->checkRXChannel();
            
            if (availableBytes > 0) {
                // Читаем команду (1 байт)
                std::vector<uint8_t> commandData(1);
                m_module->readData(commandData);
                
                uint8_t command = commandData[0];
                
                {
                    std::lock_guard<std::mutex> lock(m_statsMutex);
                    m_statistics.commandsReceived++;
                }
                
                logEvent(std::format("Received command: 0x{:02X}", command));
                
                // Обработка команды
                if (command == 0x20) {
                    // Команда версии
                    handleVersionCommand();
                }
                else if ((command & 0xF0) == 0x80 || (command & 0xF0) == 0x40) {
                    // Команда движения моторов
                    handleMotorCommand(command);
                }
                else {
                    // Неизвестная команда
                    logEvent(std::format("Unknown command: 0x{:02X}", command));
                    sendReadinessResponse(0x0A); // Общая ошибка системы
                }
            }
            else {
                // Небольшая задержка, чтобы не нагружать CPU
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error in worker loop: " << e.what() << std::endl;
            {
                std::lock_guard<std::mutex> lock(m_statsMutex);
                m_statistics.errorsOccurred++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    logEvent("Worker thread finished");
}

void MockMCU::handleVersionCommand()
{
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_statistics.versionRequests++;
    }
    
    logEvent("Handling version command");
    
    // Отправляем версию прошивки (1.2 = 0x12)
    std::vector<uint8_t> versionResponse = {0x12};
    m_module->writeData(versionResponse);
    
    logEvent("Version response sent: 1.2");
}

void MockMCU::handleMotorCommand(uint8_t commandByte)
{
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_statistics.motorCommands++;
    }
    
    bool isSynchronous = (commandByte & 0xF0) == 0x80;
    size_t motorCount = commandByte & 0x0F;
    
    logEvent(std::format("Handling motor command: {} mode, {} motors", 
                        isSynchronous ? "synchronous" : "asynchronous", motorCount));
    
    // Валидация команды
    if (!ProtocolHandler::validateMotorCommand(commandByte, motorCount)) {
        logEvent("Invalid motor command");
        sendReadinessResponse(0x01); // Некорректное количество моторов
        return;
    }
    
    // Отправляем подтверждение готовности
    sendReadinessResponse(0x00); // OK
    
    // Ждем данные моторов
    std::vector<uint8_t> motorData(motorCount * 16); // 16 байт на мотор
    m_module->readData(motorData);
    
    // Парсим данные моторов
    auto motors = ProtocolHandler::parseMotorData(motorData, motorCount);
    
    logEvent(std::format("Received data for {} motors", motors.size()));
    
    // Симулируем обработку моторов
    uint8_t result = simulateMotorProcessing(motorCount, isSynchronous);
    
    // Отправляем результат выполнения
    sendExecutionResponse(result);
    
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_statistics.commandsProcessed++;
    }
}

void MockMCU::sendReadinessResponse(uint8_t status)
{
    std::vector<uint8_t> response = {status};
    m_module->writeData(response);
    
    if (status == 0x00) {
        logEvent("Readiness response sent: OK");
    } else {
        logEvent(std::format("Readiness response sent: ERROR 0x{:02X}", status));
    }
}

void MockMCU::sendExecutionResponse(uint8_t status)
{
    std::vector<uint8_t> response = {status};
    m_module->writeData(response);
    
    if (status == 0xFF) {
        logEvent("Execution response sent: SUCCESS");
    } else {
        logEvent(std::format("Execution response sent: ERROR 0x{:02X}", status));
    }
}

uint8_t MockMCU::simulateMotorProcessing(size_t motorCount, bool isSynchronous)
{
    logEvent(std::format("Simulating motor processing: {} motors, {} mode", 
                        motorCount, isSynchronous ? "synchronous" : "asynchronous"));
    
    // Симуляция времени обработки
    std::this_thread::sleep_for(std::chrono::milliseconds(100 + motorCount * 50));
    
    // В реальном MCU здесь была бы обработка моторов
    // Для mock-MCU просто возвращаем успех
    logEvent("Motor processing simulation completed successfully");
    return 0xFF; // Успешное выполнение
}

void MockMCU::logEvent(const std::string& message)
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::tm* tm = std::localtime(&time_t);
    
    std::cout << std::format("[{:02d}:{:02d}:{:02d}.{:03d}] MockMCU: {}\n",
                            tm->tm_hour,
                            tm->tm_min,
                            tm->tm_sec,
                            ms.count(),
                            message);
}
