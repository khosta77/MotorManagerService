#include "user_core.hpp"

UserCore::~UserCore() {}

void UserCore::Init()
{
    std::vector<uint8_t> data = {0b00100000};
    module_->writeData(data);
    module_->readData(data);
    version_ = data[0] / 100.0f;
    std::cout << std::format("\nSquid version: {}\n", version_);
}

#if 0
BOOST_FUSION_DEFINE_STRUCT(
    (mms), MotorSettings,
    (int, number)
    (uint32_t, acceleration)
    (uint32_t, maxSpeed)
    (int32_t, step)
)

BOOST_FUSION_DEFINE_STRUCT(
    (mms), MotorsSettings,
    (std::string, mode)  // "synchronous" | "asynchronous"
    (std::vector<mcs::MotorSettings>, motors)
)

BOOST_FUSION_DEFINE_STRUCT(
    (pkg), Status,
    (std::string, what)
    (std::string, subMessage)
    (uint32_t, status)
)
#endif

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
        merr_.subMessage = "";
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
        merr_.subMessage = "";
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

void UserCore::Process(const int fd, const std::string &name, const std::string &message)
{
    uinfo u = {fd, name};
    auto messageIn_ = deserializeMessage(u, message); // pkg::Message
    if (!messageIn_.has_value())
        return;

    auto manager_ = deserializeManager(u, messageIn_.value().text); // mms::Manager
    if (!manager_.has_value())
        return;

    auto it = methods.find(manager_.value().command);
    if (it == methods.end())
    {
        // TODO(khosta77): #002
        return;
    }

    if (it != methods.end())
    {
        (this->*(it->second))(u, manager_.value().message); // Вызов метода через указатель
    }
}

void UserCore::Launch() {}

void UserCore::Stop() {}

void UserCore::version(const uinfo &u, const std::string &message)
{
    (void)u;
    (void)message;
}

void UserCore::moving(const uinfo &u, const std::string &message)
{
    auto motorsSetings_ = deserializeMotorsSettings(u, message);
    if (!motorsSetings_.has_value())
        return;

    if (checkMode(u, motorsSetings_.value()) || checkMotors(u, motorsSetings_.value()))
        return;
}

void UserCore::reconnect(const uinfo &u, const std::string &message)
{
    (void)u;
    (void)message;
}

void UserCore::disconnect(const uinfo &u, const std::string &message)
{
    (void)u;
    (void)message;
}

void UserCore::listconnect(const uinfo &u, const std::string &message)
{
    (void)u;
    (void)message;
}

