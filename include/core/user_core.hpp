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
        , module_(std::move(module))
        , version_(0.0f)
    {}
    explicit UserCore(std::unique_ptr<IModule> module)
        : UserCore(std::move(module), std::make_unique<Socket>())
    {}
    UserCore(const UserCore &) = delete;
    UserCore(UserCore &&) = delete;
    ~UserCore() override;

    UserCore &operator=(const UserCore &) = delete;
    UserCore &operator=(UserCore &&) = delete;

    void Init() override;
    void Process(const int fd, const std::string &name, const std::string &message) override;
    void Launch() override;
    void Stop() override;

private:
    using uinfo = std::pair<int, std::string>;
    using MethodPtr = void (UserCore::*)(const uinfo &, const std::string &);

    std::unique_ptr<IModule> module_;
    float version_;

    std::unordered_map<std::string, MethodPtr> methods = {
        {"version", &UserCore::version},
        {"moving", &UserCore::moving},
        {"reconnect", &UserCore::reconnect},
        {"disconnect", &UserCore::disconnect},
        {"listconnect", &UserCore::listconnect}};

    /**
     * @brief Получает информацию о версии прошивки микроконтроллера
     * 
     * Метод отправляет команду запроса версии прошивки в микроконтроллер,
     * получает ответ в виде одного байта, где первые 4 бита - целая часть версии,
     * а вторые 4 бита - дробная часть. Формирует структуру mms::Version с именем "Squid"
     * и отправляет её клиенту через сокет.
     * 
     * @param u Информация о пользователе (дескриптор сокета и имя)
     * @param message Должна быть пустой строкой, иначе возвращается ошибка
     */
    void version(const uinfo &u, const std::string &message);
    void moving(const uinfo &u, const std::string &message);
    void reconnect(const uinfo &u, const std::string &message);
    void disconnect(const uinfo &u, const std::string &message);
    void listconnect(const uinfo &u, const std::string &message);

    std::optional<pkg::Message> deserializeMessage(const uinfo &, const std::string &);
    std::optional<mms::Manager> deserializeManager(const uinfo &, const std::string &);
    std::optional<mms::MotorsSettings> deserializeMotorsSettings(const uinfo &, const std::string &);
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
};

#endif // TRANSFORMATIONCORE_HPP_
