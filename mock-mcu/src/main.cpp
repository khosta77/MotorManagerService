#include "mock_mcu.hpp"
#include <iostream>
#include <csignal>
#include <atomic>
#include <format>
#include <thread>
#include <chrono>

// Глобальная переменная для корректного завершения
std::atomic<bool> g_running{true};
std::unique_ptr<MockMCU> g_mockMCU;

/**
 * @brief Обработчик сигналов для корректного завершения
 */
void signalHandler(int signal)
{
    std::cout << std::format("\nReceived signal {}, shutting down...\n", signal);
    g_running = false;
    
    if (g_mockMCU) {
        g_mockMCU->stop();
    }
}

/**
 * @brief Вывод справки по использованию
 */
void printUsage(const char* programName)
{
    std::cout << std::format(
        "MockMCU - Имитатор микроконтроллера для тестирования MotorControlService\n\n"
        "Использование: {} [OPTIONS]\n\n"
        "Опции:\n"
        "  -d, --device ID     ID устройства FT232RL (по умолчанию: 1)\n"
        "  -h, --help          Показать эту справку\n"
        "  -v, --verbose       Подробный вывод\n"
        "  -s, --stats         Показывать статистику каждые 5 секунд\n\n"
        "Примеры:\n"
        "  {}                  # Запуск с устройством ID=1\n"
        "  {} -d 0             # Запуск с устройством ID=0\n"
        "  {} -d 1 -s          # Запуск с показом статистики\n\n"
        "Протокол:\n"
        "  MockMCU имитирует микроконтроллер, который:\n"
        "  - Принимает команды от MotorControlService\n"
        "  - Обрабатывает команды version() и moving()\n"
        "  - Отправляет ответы согласно протоколу\n\n",
        programName, programName, programName, programName
    );
}

/**
 * @brief Вывод статистики
 */
void printStatistics(const MockMCU::Statistics& stats)
{
    std::cout << std::format(
        "\n=== Статистика MockMCU ===\n"
        "Команд получено:     {}\n"
        "Команд обработано:   {}\n"
        "Ошибок произошло:    {}\n"
        "Запросов версии:     {}\n"
        "Команд моторов:      {}\n"
        "========================\n",
        stats.commandsReceived,
        stats.commandsProcessed,
        stats.errorsOccurred,
        stats.versionRequests,
        stats.motorCommands
    );
}

int main(int argc, char* argv[])
{
    // Настройка обработчиков сигналов
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Параметры по умолчанию
    int deviceId = 1;
    bool verbose = false;
    bool showStats = false;
    
    // Парсинг аргументов командной строки
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        else if (arg == "-d" || arg == "--device") {
            if (i + 1 < argc) {
                try {
                    deviceId = std::stoi(argv[++i]);
                } catch (const std::exception& e) {
                    std::cerr << "Ошибка: неверный ID устройства: " << argv[i] << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Ошибка: не указан ID устройства после -d" << std::endl;
                return 1;
            }
        }
        else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        }
        else if (arg == "-s" || arg == "--stats") {
            showStats = true;
        }
        else {
            std::cerr << "Неизвестный аргумент: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    std::cout << std::format(
        "MockMCU v1.0 - Имитатор микроконтроллера\n"
        "Устройство FT232RL: {}\n"
        "Режим: {}\n"
        "Статистика: {}\n\n",
        deviceId,
        verbose ? "подробный" : "обычный",
        showStats ? "включена" : "отключена"
    );
    
    // Создание и инициализация MockMCU
    g_mockMCU = std::make_unique<MockMCU>();
    
    if (!g_mockMCU->initialize(deviceId)) {
        std::cerr << "Ошибка инициализации MockMCU" << std::endl;
        return 1;
    }
    
    // Запуск MockMCU
    g_mockMCU->start();
    
    std::cout << "MockMCU запущен. Нажмите Ctrl+C для остановки.\n" << std::endl;
    
    // Основной цикл
    auto lastStatsTime = std::chrono::steady_clock::now();
    
    while (g_running && g_mockMCU->isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Показ статистики каждые 5 секунд
        if (showStats) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastStatsTime).count() >= 5) {
                printStatistics(g_mockMCU->getStatistics());
                lastStatsTime = now;
            }
        }
    }
    
    // Корректное завершение
    std::cout << "\nЗавершение работы MockMCU..." << std::endl;
    
    if (g_mockMCU) {
        auto finalStats = g_mockMCU->getStatistics();
        printStatistics(finalStats);
        
        g_mockMCU->stop();
    }
    
    std::cout << "MockMCU остановлен." << std::endl;
    return 0;
}
