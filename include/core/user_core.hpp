#ifndef TRANSFORMATIONCORE_HPP_
#define TRANSFORMATIONCORE_HPP_

#include <bit>

#include "i_module.hpp"
#include "network_serializer.hpp"
#include "dataframe.hpp"

#define MOTOR_COUNT       16
#define MOTOR_CELLS_COUNT 4


/*
 * +-+-+-+-+-+-+-+-+---------------------------------------------------------+
 * |0|1|2|3|4|5|6|7|что?                                                     |
 * +-+-+-+-+-+-+-+-+---------------------------------------------------------+
 * |1|0|0|0|X|X|X|X|Запуска одновременно всех ШД, на которые пришла установка|
 * +-+-+-+-+-+-+-+-+---------------------------------------------------------+
 * |0|1|0|0|X|X|X|X|Запуск по мере готовности, в асинхронном режиме          |
 * +-+-+-+-+-+-+-+-+---------------------------------------------------------+
 * |0|0|1|0|X|X|X|X|Запросить информацию о версии прошивки                   |
 * +-+-+-+-+-+-+-+-+---------------------------------------------------------+
 * */

template <typename Derived>
class UserCore : public ICore, public NetworkSerializer
{
public:
    UserCore() = delete;
    UserCore(std::unique_ptr<IModule<Derived>> module, std::unique_ptr<ISocket> socket)
        : ICore("MotorManagerService")
        , NetworkSerializer(std::move(socket))
        , module_(std::move(module))
        , version_(0.0f)
    {}
    explicit UserCore(std::unique_ptr<IModule<Derived>> module)
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
    using MethodPtr = void (A::*)(const uinfo &, const std::string &);

    std::unique_ptr<IModule<Derived>> module_;
    float version_;

    map<string, MethodPtr> methods = {
        {"version", &UserCore::version},
        {"moving", &UserCore::moving},
        {"reconnect", &UserCore::reconnect},
        {"disconnect", &UserCore::disconnect},
        {"listconnect", &UserCore::listconnect}};

    void version(const uinfo &u, const std::string &message);
    void moving(const uinfo &u, const std::string &message);
    void reconnect(const uinfo &u, const std::string &message);
    void disconnect(const uinfo &u, const std::string &message);
    void listconnect(const uinfo &u, const std::string &message);

    std::optional<pkg::Message> deserializeMessage(const uinfo &, const std::string &);
    std::optional<mms::Manager> deserializeManager(const uinfo &, const std::string &);
    std::optional<mms::MotorsSettings> deserializeMotorsSettings(const uinfo &, const pkg::Message &);
    bool checkMode(const uinfo &, const mms::MotorsSettings &);
    bool checkMotors(const uinfo &, const mms::MotorsSettings &);
};

#endif // TRANSFORMATIONCORE_HPP_
