#ifndef PROTOCOL_HANDLER_HPP_
#define PROTOCOL_HANDLER_HPP_

#include <cstdint>
#include <vector>
#include <string>

/**
 * @brief Обработчик протокола взаимодействия с MCU
 * 
 * Этот класс содержит логику обработки различных команд протокола
 * согласно документации device_interface.md
 */
class ProtocolHandler
{
public:
    /**
     * @brief Структура данных мотора для обработки
     */
    struct MotorData {
        uint32_t number;
        uint32_t acceleration;
        uint32_t maxSpeed;
        int32_t step;
    };
    
    /**
     * @brief Результат обработки команды
     */
    struct CommandResult {
        bool success;
        uint8_t errorCode;
        std::string description;
    };

    /**
     * @brief Обработка команды версии
     * @return Результат обработки
     */
    static CommandResult handleVersionCommand();
    
    /**
     * @brief Обработка команды движения моторов
     * @param commandByte Байт команды
     * @param motorData Данные моторов
     * @return Результат обработки
     */
    static CommandResult handleMotorCommand(uint8_t commandByte, const std::vector<MotorData>& motorData);
    
    /**
     * @brief Парсинг данных моторов из байтового массива
     * @param data Байтовый массив с данными
     * @param motorCount Количество моторов
     * @return Вектор данных моторов
     */
    static std::vector<MotorData> parseMotorData(const std::vector<uint8_t>& data, size_t motorCount);
    
    /**
     * @brief Валидация команды движения
     * @param commandByte Байт команды
     * @param motorCount Количество моторов
     * @return true если команда валидна
     */
    static bool validateMotorCommand(uint8_t commandByte, size_t motorCount);
    
    /**
     * @brief Получение описания ошибки по коду
     * @param errorCode Код ошибки
     * @return Описание ошибки
     */
    static std::string getErrorDescription(uint8_t errorCode);

private:
    /**
     * @brief Симуляция обработки одного мотора
     * @param motor Данные мотора
     * @return Код результата (0xFF = успех, другие = ошибка)
     */
    static uint8_t simulateSingleMotorProcessing(const MotorData& motor);
    
    /**
     * @brief Проверка валидности параметров мотора
     * @param motor Данные мотора
     * @return true если параметры валидны
     */
    static bool validateMotorParameters(const MotorData& motor);
};

#endif // PROTOCOL_HANDLER_HPP_
