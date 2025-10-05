#ifndef MOCK_MCU_HPP_
#define MOCK_MCU_HPP_

#include <memory>
#include <vector>
#include <cstdint>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "i_module.hpp"

/**
 * @brief Mock MCU - имитатор микроконтроллера для тестирования протокола
 * 
 * Этот класс имитирует поведение реального микроконтроллера, который:
 * 1. Принимает команды от основного сервиса через FT232RL
 * 2. Обрабатывает команды согласно протоколу
 * 3. Отправляет ответы обратно в сервис
 */
class MockMCU
{
public:
    MockMCU();
    ~MockMCU();

    /**
     * @brief Инициализация mock-MCU
     * @param deviceId ID устройства FT232RL для подключения
     * @return true если инициализация успешна
     */
    bool initialize(int deviceId);

    /**
     * @brief Запуск обработки команд
     */
    void start();

    /**
     * @brief Остановка обработки команд
     */
    void stop();

    /**
     * @brief Проверка состояния работы
     * @return true если MCU работает
     */
    bool isRunning() const;

    /**
     * @brief Получение статистики работы
     */
    struct Statistics {
        uint32_t commandsReceived = 0;
        uint32_t commandsProcessed = 0;
        uint32_t errorsOccurred = 0;
        uint32_t versionRequests = 0;
        uint32_t motorCommands = 0;
    };
    
    Statistics getStatistics() const;

private:
    std::unique_ptr<IModule> m_module;
    std::atomic<bool> m_running;
    std::atomic<bool> m_initialized;
    
    mutable std::mutex m_statsMutex;
    Statistics m_statistics;
    
    std::thread m_workerThread;
    
    /**
     * @brief Основной цикл обработки команд
     */
    void workerLoop();
    
    /**
     * @brief Обработка команды версии (0x20)
     */
    void handleVersionCommand();
    
    /**
     * @brief Обработка команды движения моторов
     * @param commandByte Байт команды (0x8N или 0x4N)
     */
    void handleMotorCommand(uint8_t commandByte);
    
    /**
     * @brief Отправка ответа готовности
     * @param status Код статуса (0x00 = OK, другие = ошибка)
     */
    void sendReadinessResponse(uint8_t status);
    
    /**
     * @brief Отправка ответа выполнения
     * @param status Код статуса (0xFF = успех, другие = ошибка)
     */
    void sendExecutionResponse(uint8_t status);
    
    /**
     * @brief Симуляция обработки моторов
     * @param motorCount Количество моторов
     * @param isSynchronous Синхронный ли режим
     * @return Код результата выполнения
     */
    uint8_t simulateMotorProcessing(size_t motorCount, bool isSynchronous);
    
    /**
     * @brief Логирование событий
     */
    void logEvent(const std::string& message);
};

#endif // MOCK_MCU_HPP_
