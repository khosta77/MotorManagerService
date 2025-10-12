#include "user_core.hpp"
#include <thread>
#include <chrono>
#include <algorithm>

using namespace std::chrono_literals;

UserCore::~UserCore() {}

void UserCore::Init()
{
#if 0
    std::vector<uint8_t> data = {0b00100000};
    m_module->writeData(data);
    m_module->readData(data);
    m_version = data[0] / 100.0f;
    std::cout << std::format("\nSquid version: {}\n", m_version);
#endif
}

std::optional<pkg::Message> UserCore::deserializeMessage(const uinfo &u, const std::string &message)
{
    pkg::Message message_in;
    try
    {
        message_in = deserialize<pkg::Message>(message);
    }
    catch (...)
    {
        pkg::Status merr_;
        merr_.status = 40401; // TODO(khosta77): #001
        merr_.what = std::format("[{}]: The message is correct({})", u.second, message);
        merr_.subMessage = "pkg::Message";
        writeToSock(u.first, serialize(merr_));
        return {};
    }
    return message_in;
}

std::optional<mms::Manager> UserCore::deserializeManager(const uinfo &u, const std::string &text)
{
    mms::Manager manager;
    try
    {
        manager = deserialize<mms::Manager>(text);
    }
    catch (...)
    {
        pkg::Status merr_;
        merr_.status = 40401; // TODO(khosta77): #001
        merr_.what = std::format("[{}]: The message is correct({})", u.second, text);
        merr_.subMessage = "mms::Manager";
        writeToSock(u.first, serialize(merr_));
        return {};
    }
    return manager;
}

std::optional<mms::MotorsSettings> UserCore::deserializeMotorsSettings(
    const uinfo &u,
    const std::string &message)
{
    mms::MotorsSettings motorsSetings_;
    try
    {
        motorsSetings_ = deserialize<mms::MotorsSettings>(message);
    }
    catch (...)
    {
        pkg::Status merr_;
        merr_.status = 40402; // TODO(khosta77): #001
        merr_.what = std::format("[{}]: The \"MotorsSettings\" is correct({})", u.second, message);
        merr_.subMessage = "";
        writeToSock(u.first, serialize(merr_));
        return {};
    }
    return motorsSetings_;
}

std::optional<mms::Device> UserCore::deserializeDevice(const uinfo &u, const std::string &message)
{
    mms::Device device_;
    try
    {
        device_ = deserialize<mms::Device>(message);
    }
    catch (...)
    {
        pkg::Status merr_;
        merr_.status = 40403; // TODO(khosta77): #001
        merr_.what = std::format("[{}]: The \"Device\" is correct({})", u.second, message);
        merr_.subMessage = "";
        writeToSock(u.first, serialize(merr_));
        return {};
    }
    return device_;
}

bool UserCore::checkMode(const uinfo &u, const mms::MotorsSettings &motorsSetings_)
{
    if (motorsSetings_.mode == "synchronous" || motorsSetings_.mode == "asynchronous")
        return false;

    pkg::Status merr_;
    merr_.status = 40501; // TODO(khosta77): #001
    merr_.what = std::format("[{}]: The \"mode\" is correct({})", u.second, motorsSetings_.mode);
    merr_.subMessage = "";
    writeToSock(u.first, serialize(merr_));
    return true;
}

bool UserCore::checkMotors(const uinfo &u, const mms::MotorsSettings &motorsSettings_)
{
    if (motorsSettings_.motors.size() > 10)
    {
        pkg::Status merr_;
        merr_.status = 40502; // TODO: #001
        merr_.what = std::format(
            "[{}]: Motors array size exceeds limit ({} > 10)",
            u.second,
            motorsSettings_.motors.size());
        merr_.subMessage = "";
        writeToSock(u.first, serialize(merr_));
        return true;
    }

    for (size_t i = 0; i < motorsSettings_.motors.size(); ++i)
    {
        const auto &motor = motorsSettings_.motors[i];

        if (motor.number < 1 || motor.number > 10)
        {
            pkg::Status merr_;
            merr_.status = 40503; // TODO: #001
            merr_.what = std::format(
                "[{}]: Motor #{} has invalid number ({}), must be 1-10",
                u.second,
                i + 1,
                motor.number);
            merr_.subMessage = "";
            writeToSock(u.first, serialize(merr_));
            return true;
        }

        if (motor.acceleration == 0)
        {
            pkg::Status merr_;
            merr_.status = 40504; // TODO: #001
            merr_.what = std::format("[{}]: Motor #{} has zero acceleration", u.second, motor.number);
            merr_.subMessage = "";
            writeToSock(u.first, serialize(merr_));
            return true;
        }

        if (motor.maxSpeed == 0)
        {
            pkg::Status merr_;
            merr_.status = 40505; // TODO: #001
            merr_.what = std::format("[{}]: Motor #{} has zero max speed", u.second, motor.number);
            merr_.subMessage = "";
            writeToSock(u.first, serialize(merr_));
            return true;
        }
    }

    return false;
}

bool UserCore::checkEmptyMessage(const uinfo &u, const std::string &message)
{
    if (!message.empty())
    {
        pkg::Status merr_;
        merr_.status = 40506; // TODO: #001
        merr_.what =
            std::format("[{}]: The message should be empty for version command, got: {}", u.second, message);
        merr_.subMessage = "";
        writeToSock(u.first, serialize(merr_));
        return true;
    }
    return false;
}

bool UserCore::checkConnection(const uinfo &u)
{
    if (!m_module->isConnected())
    {
        pkg::Status merr_;
        merr_.status = 40507; // TODO: #001
        merr_.what = std::format("[{}]: Module is not connected", u.second);
        merr_.subMessage = "";
        writeToSock(u.first, serialize(merr_));
        return true;
    }
    return false;
}

bool UserCore::checkDeviceId(const uinfo &u, int deviceId)
{
    if (deviceId < 0)
    {
        pkg::Status merr_;
        merr_.status = 40509; // TODO: #001
        merr_.what = std::format("[{}]: Device id must be >= 0, got {}", u.second, deviceId);
        merr_.subMessage = "";
        writeToSock(u.first, serialize(merr_));
        return true;
    }
    return false;
}

bool UserCore::checkConnectResult(const uinfo &u, int deviceId, bool ok)
{
    if (!ok)
    {
        pkg::Status merr_;
        merr_.status = 40510; // TODO: #001
        merr_.what = std::format("[{}]: Failed to connect device {}", u.second, deviceId);
        merr_.subMessage = "";
        writeToSock(u.first, serialize(merr_));
        return true;
    }
    return false;
}

void UserCore::Process(const int fd, const std::string &name, const std::string &message)
{
    uinfo u = {fd, name};
    auto messageIn_ = deserializeMessage(u, message); // pkg::Message
    if (!messageIn_.has_value())
        return;

    auto manager_ = deserializeManager(u, messageIn_.value().text); // mms::Manager
    if (!manager_.has_value())
        return;

    auto it = m_methods.find(manager_.value().command);
    if (it == m_methods.end())
    {
        // TODO(khosta77): #002
        return;
    }

    (this->*(it->second))(u, manager_.value().message); // Вызов метода через указатель
}

void UserCore::Launch() {}

void UserCore::Stop() {}

void UserCore::version(const uinfo &u, const std::string &message)
{
    if (checkEmptyMessage(u, message))
        return;

    if (checkConnection(u))
        return;

    std::vector<uint8_t> data = {0b00100000}; // Команда запроса версии прошивки
    m_module->writeData(data);
    data[0] = 0x00;
    std::this_thread::sleep_for(100ms);
    m_module->readData(data);

    uint8_t versionByte = data[0];
    uint8_t integerPart = (versionByte >> 4) & 0x0F; // Первые 4 бита
    uint8_t decimalPart = versionByte & 0x0F;        // Вторые 4 бита

    mms::Version versionInfo;
    versionInfo.version = static_cast<float>(integerPart) + static_cast<float>(decimalPart) / 10.0f;
    versionInfo.name = "Squid";

    pkg::Status response;
    response.status = 0;
    response.subMessage = serialize(versionInfo);
    response.what = "mms::Version";

    writeToSock(u.first, serialize(response));
}

void UserCore::moving(const uinfo &u, const std::string &message)
{
    if (checkConnection(u))
        return;

    auto motorsSettings_ = deserializeMotorsSettings(u, message);
    if (!motorsSettings_.has_value())
        return;

    if (checkMode(u, motorsSettings_.value()) || checkMotors(u, motorsSettings_.value()))
        return;

    const auto &settings = motorsSettings_.value();
    const size_t motorCount = settings.motors.size();

    uint8_t commandByte;
    if (settings.mode == "synchronous")
    {
        commandByte = 0x80 | static_cast<uint8_t>(motorCount); // 0x8N
    }
    else
    {                                                          // asynchronous
        commandByte = 0x40 | static_cast<uint8_t>(motorCount); // 0x4N
    }

    std::vector<uint8_t> commandData = {commandByte};
    m_module->writeData(commandData);
    std::vector<uint8_t> readinessResponse(1, 0);

    size_t timeoutMs = 5000; // 5 секунд
    size_t elapsedMs = 0;
    const size_t checkIntervalMs = 100;
    while (elapsedMs < timeoutMs)
    {
        size_t availableBytes = m_module->checkRXChannel();
        std::cout << std::format("\tсообщение: {}\n", availableBytes);
        if (availableBytes >= 1)
        {
            m_module->readData(readinessResponse);
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
        elapsedMs += checkIntervalMs;
    }

    uint8_t readinessCode = readinessResponse[0];
    if (readinessCode != 0x00)
    {
        pkg::Status errorResponse;
        errorResponse.status = 40512; // MCU readiness error
        errorResponse.what = std::format("[{}][40512]: MCU readiness error: {}", u.second, readinessCode);
        errorResponse.subMessage = "";
        writeToSock(u.first, serialize(errorResponse));
        return;
    }

    std::vector<uint8_t> motorData;
    motorData.reserve(motorCount * 16); // 4 параметра × 4 байта на мотор

    for (const auto &motor : settings.motors)
    {
        uint32_t number = static_cast<uint32_t>(motor.number);
        motorData.insert(
            motorData.end(),
            reinterpret_cast<uint8_t *>(&number),
            reinterpret_cast<uint8_t *>(&number) + 4);

        uint32_t acceleration = motor.acceleration;
        motorData.insert(
            motorData.end(),
            reinterpret_cast<uint8_t *>(&acceleration),
            reinterpret_cast<uint8_t *>(&acceleration) + 4);

        uint32_t maxSpeed = motor.maxSpeed;
        motorData.insert(
            motorData.end(),
            reinterpret_cast<uint8_t *>(&maxSpeed),
            reinterpret_cast<uint8_t *>(&maxSpeed) + 4);

        uint32_t step = static_cast<uint32_t>(motor.step);
        motorData.insert(
            motorData.end(),
            reinterpret_cast<uint8_t *>(&step),
            reinterpret_cast<uint8_t *>(&step) + 4);
    }

    m_module->writeData(motorData);
    std::this_thread::sleep_for(200ms);

    std::vector<uint8_t> completionResponse(2);

    // Простая реализация таймаута через проверку доступных данных
    // В реальной реализации здесь должен быть более сложный механизм таймаута
    elapsedMs = 0;
    while (elapsedMs < timeoutMs)
    {
        size_t availableBytes = m_module->checkRXChannel();
        std::cout << std::format("\t\tсообщение: {}\n", availableBytes);
        if (availableBytes >= 2)
        {
            m_module->readData(completionResponse);
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
        elapsedMs += checkIntervalMs;
    }

    for (int i = 0; i < completionResponse.size(); ++i)
    {
        printf("%02x ", completionResponse[i]);
    }
    printf("\n");

    if (elapsedMs >= timeoutMs)
    {
        // Таймаут ожидания ответа от MCU
        pkg::Status errorResponse;
        errorResponse.status = 40511; // Timeout waiting for MCU response
        errorResponse.what = std::format("[{}]: Timeout waiting for MCU response", u.second);
        errorResponse.subMessage = "";
        writeToSock(u.first, serialize(errorResponse));
        return;
    }

    uint8_t completionCode = completionResponse[1];
    if (completionCode != 0xFF)
    {
        // Ошибка выполнения на MCU
        pkg::Status errorResponse;
        errorResponse.status = 40513; // MCU execution error
        errorResponse.what =
            std::format("[{}][40513]: MCU execution error: 0x{:02X}", u.second, completionCode);
        errorResponse.subMessage = "";
        writeToSock(u.first, serialize(errorResponse));
        return;
    }

    pkg::Status successResponse;
    successResponse.status = 0;
    successResponse.what = "";
    successResponse.subMessage = "";
    writeToSock(u.first, serialize(successResponse));
}

void UserCore::reconnect(const uinfo &u, const std::string &message)
{
    if (m_module->isConnected())
    {
        pkg::Status merr_;
        merr_.status = 40512; // TODO: #001
        merr_.what = std::format("[{}]: Module is connecting", u.second);
        merr_.subMessage = "";
        writeToSock(u.first, serialize(merr_));
        return;
    }

    auto device_ = deserializeDevice(u, message);
    if (!device_.has_value())
        return;

    if (checkDeviceId(u, device_.value().deviceId))
        return;

    bool ok = m_module->connect(device_.value().deviceId);
    if (checkConnectResult(u, device_.value().deviceId, ok))
        return;

    pkg::Status ok_;
    ok_.status = 0;
    ok_.what = "";
    ok_.subMessage = "";
    writeToSock(u.first, serialize(ok_));
}

void UserCore::disconnect(const uinfo &u, const std::string &message)
{
    if (checkEmptyMessage(u, message))
        return;

    if (checkConnection(u))
        return;

    m_module->disconnect();

    pkg::Status ok_;
    ok_.status = 0;
    ok_.what = "";
    ok_.subMessage = "";
    writeToSock(u.first, serialize(ok_));
}

void UserCore::listconnect(const uinfo &u, const std::string &message)
{
    if (checkEmptyMessage(u, message))
        return;

    mms::ListConnect list;
    auto rawDevices = m_module->listComs();

    // Очистка и валидация строк для корректного UTF-8
    for (const auto &device : rawDevices)
    {
        std::string cleanDevice = device;

        // Удаляем невалидные UTF-8 символы (символы с кодом > 127)
        cleanDevice.erase(
            std::remove_if(
                cleanDevice.begin(),
                cleanDevice.end(),
                [](char c) { return static_cast<unsigned char>(c) > 127; }),
            cleanDevice.end());

        // Дополнительная проверка на наличие обратных кавычек (0x60)
        cleanDevice.erase(
            std::remove_if(
                cleanDevice.begin(),
                cleanDevice.end(),
                [](char c) {
                    return c == 0x60; // Удаляем обратные кавычки
                }),
            cleanDevice.end());

        // Если строка не пустая после очистки, добавляем её
        if (!cleanDevice.empty())
        {
            list.listConnect.push_back(cleanDevice);
        }
    }

    // Если нет устройств, добавляем заглушку
    if (list.listConnect.empty())
    {
        list.listConnect.push_back("No devices found");
    }

    pkg::Status ok_;
    ok_.status = 0;
    ok_.what = "mms::ListConnect";
    ok_.subMessage = serialize(list);
    writeToSock(u.first, serialize(ok_));
}

