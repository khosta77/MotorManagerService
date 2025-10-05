#ifndef TRANSFORMATIONCORE_HPP_
#define TRANSFORMATIONCORE_HPP_

#include <bit>

#include "i_module.hpp"
#include "network_serializer.hpp"
#include "dataframe.hpp"

/*
 * +-+-+-+-+-+-+-+-+---------------------------------------------------------+
 * |0|1|2|3|4|5|6|7|Биты для отправки в микроконтрллер                       |
 * +-+-+-+-+-+-+-+-+---------------------------------------------------------+
 * |1|0|0|0|X|X|X|X|Запуска одновременно всех ШД, на которые пришла установка|
 * +-+-+-+-+-+-+-+-+---------------------------------------------------------+
 * |0|1|0|0|X|X|X|X|Запуск по мере готовности, в асинхронном режиме          |
 * +-+-+-+-+-+-+-+-+---------------------------------------------------------+
 * |0|0|1|0|X|X|X|X|Запросить информацию о версии прошивки                   |
 * +-+-+-+-+-+-+-+-+---------------------------------------------------------+
 * command: [] version()
 *          [] moving(MotorsSettings)
 *          [] reconnect(id)
 *          [] disconnect()
 *          [] listconnect()
 * */

class UserCore : public ICore, public NetworkSerializer
{
public:
    UserCore() = delete;
    UserCore(std::unique_ptr<IModule> module, std::unique_ptr<ISocket> socket)
        : ICore("MotorManagerService")
        , NetworkSerializer(std::move(socket))
        , m_module(std::move(module))
        , m_version(0.0f)
    {}
    explicit UserCore(std::unique_ptr<IModule> module)
        : UserCore(std::move(module), std::make_unique<Socket>())
    {}
    UserCore(const UserCore &) = delete;
    UserCore(UserCore &&) = delete;
    ~UserCore() override;

    UserCore &operator=(const UserCore &) = delete;
    UserCore &operator=(UserCore &&) = delete;

    /**
     * @brief Инициализация ядра сервиса
     * Выполняет начальный запрос версии устройства для верификации связи.
     */
    void Init() override;
    /**
     * @brief Обработка входящего сообщения от клиента
     * @param fd Дескриптор сокета клиента
     * @param name Имя клиента
     * @param message Сериализованное сообщение `pkg::Message`
     */
    void Process(const int fd, const std::string &name, const std::string &message) override;
    /**
     * @brief Запуск сервиса (при наличии фоновой логики)
     */
    void Launch() override;
    /**
     * @brief Остановка сервиса (освобождение ресурсов)
     */
    void Stop() override;

private:
    using uinfo = std::pair<int, std::string>;
    using MethodPtr = void (UserCore::*)(const uinfo &, const std::string &);

    std::unique_ptr<IModule> m_module;
    float m_version;

    std::unordered_map<std::string, MethodPtr> m_methods = {
        {"version", &UserCore::version},
        {"moving", &UserCore::moving},
        {"reconnect", &UserCore::reconnect},
        {"disconnect", &UserCore::disconnect},
        {"listconnect", &UserCore::listconnect}};

    /**
     * @brief Команда version()
     * 
     * Отправляет запрос версии прошивки в микроконтроллер и возвращает
     * структуру `mms::Version` (через `pkg::Status`). Версия кодируется одним байтом:
     * старшие 4 бита — целая часть, младшие 4 бита — дробная часть (x.y).
     * 
     * Правила и проверки:
     * - Сообщение `message` обязано быть пустым, иначе ошибка `40506`.
     * - Требуется активное соединение с модулем, иначе ошибка `40507`.
     * 
     * Ответ:
     * - `status = 0`, `what = "mms::Version"`, `subMessage = serialize(mms::Version)`
     * 
     * @param u Информация о пользователе (дескриптор сокета и имя)
     * @param message Должна быть пустой строкой
     */
    void version(const uinfo &u, const std::string &message);
    /**
     * @brief Команда moving(MotorsSettings)
     * 
     * Принимает сериализованные настройки двигателей `mms::MotorsSettings`,
     * валидирует режим работы ("synchronous"|"asynchronous") и параметры каждого двигателя
     * (кол-во не более 10, номер 1..10, acceleration>0, maxSpeed>0). В дальнейшем
     * выполняет соответствующую команду на модуле (реализация управления — отдельно).
     * 
     * Правила и проверки:
     * - Требуется активное соединение с модулем, иначе ошибка `40507`.
     * - Ошибка десериализации настроек — `40402`.
     * - Некорректный `mode` — `40501`.
     * - Нарушение ограничений по массиву/параметрам моторов — `40502..40505`.
     * 
     * @param u Информация о пользователе
     * @param message Сериализованный `mms::MotorsSettings`
     */
    void moving(const uinfo &u, const std::string &message);
    /**
     * @brief Команда reconnect(id)
     * 
     * Принимает сериализованный `mms::Device` с `deviceId` и пытается подключиться
     * к указанному устройству.
     * 
     * Правила и проверки:
     * - Ошибка десериализации `mms::Device` — `40403`.
     * - `deviceId < 0` — ошибка `40509`.
     * - Неудачное подключение — ошибка `40510`.
     * 
     * Ответ при успехе:
     * - `status = 0`, `what = ""`, `subMessage = ""`.
     * 
     * @param u Информация о пользователе
     * @param message Сериализованный `mms::Device`
     */
    void reconnect(const uinfo &u, const std::string &message);
    /**
     * @brief Команда disconnect()
     * 
     * Отключает текущее устройство от модуля.
     * 
     * Правила и проверки:
     * - Сообщение `message` обязано быть пустым, иначе ошибка `40506`.
     * - Требуется активное соединение с модулем, иначе ошибка `40507` (через checkConnection).
     * 
     * Ответ при успехе:
     * - `status = 0`, `what = ""`, `subMessage = ""`.
     * 
     * @param u Информация о пользователе
     * @param message Должна быть пустой строкой
     */
    void disconnect(const uinfo &u, const std::string &message);
    /**
     * @brief Команда listconnect()
     * 
     * Возвращает список доступных устройств. Подключение к модулю не требуется.
     * 
     * Правила и проверки:
     * - Сообщение `message` обязано быть пустым, иначе ошибка `40506`.
     * 
     * Ответ:
     * - `status = 0`, `what = "mms::ListConnect"`, `subMessage = serialize(mms::ListConnect)`.
     * 
     * @param u Информация о пользователе
     * @param message Должна быть пустой строкой
     */
    void listconnect(const uinfo &u, const std::string &message);

    std::optional<pkg::Message> deserializeMessage(const uinfo &, const std::string &);
    std::optional<mms::Manager> deserializeManager(const uinfo &, const std::string &);
    std::optional<mms::MotorsSettings> deserializeMotorsSettings(const uinfo &, const std::string &);
    std::optional<mms::Device> deserializeDevice(const uinfo &, const std::string &);
    bool checkMode(const uinfo &, const mms::MotorsSettings &);
    bool checkMotors(const uinfo &, const mms::MotorsSettings &);
    /**
     * @brief Проверяет, что сообщение пустое
     * 
     * @param u Информация о пользователе (дескриптор сокета и имя)
     * @param message Сообщение для проверки
     * @return true если сообщение не пустое (ошибка), false если пустое (OK)
     */
    bool checkEmptyMessage(const uinfo &, const std::string &);
    /**
     * @brief Проверяет соединение с модулем
     * 
     * @param u Информация о пользователе (дескриптор сокета и имя)
     * @return true если соединения нет (ошибка), false если соединение есть (OK)
     */
    bool checkConnection(const uinfo &);
    /**
     * @brief Проверяет корректность идентификатора устройства
     * @param u Информация о пользователе
     * @param deviceId Идентификатор устройства
     * @return true если deviceId некорректен (ошибка), false если корректен (OK)
     */
    bool checkDeviceId(const uinfo &, int deviceId);
    /**
     * @brief Проверяет результат подключения к устройству
     * @param u Информация о пользователе
     * @param deviceId Идентификатор устройства
     * @param ok Результат попытки подключения
     * @return true если подключение не удалось (ошибка), false если успех (OK)
     */
    bool checkConnectResult(const uinfo &, int deviceId, bool ok);
};

#endif // TRANSFORMATIONCORE_HPP_
