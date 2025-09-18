#include "MotorControlServiceCore.hpp"

void print(uint32_t num)
{
    for (int i = 31; i >= 0; --i)
    {
        std::cout << ((num >> i) & 1);
        if (i > 0 && i % 8 == 0)
        {
            std::cout << "'";
        }
    }
}

void print(const std::vector<uint32_t> &values)
{
    if (values.size() != (MOTOR_COUNT * MOTOR_CELLS_COUNT))
        throw std::length_error(std::format(
            "[MotorControlServiceCore](print): values.size() = {}, must be {}",
            values.size(),
            (MOTOR_COUNT * MOTOR_CELLS_COUNT)));
    for (size_t i = 0; i < values.size(); i += 4)
    {
        // std::cout << ( i / 4 ) << ')';
        print(values[i]);
        std::cout << ' ';
        print(values[i + 1]);
        std::cout << ' ';
        print(values[i + 2]);
        std::cout << ' ';
        print(values[i + 3]);
        std::cout << std::endl;
        // std::cout << ' ' << values[i + 1] << ' ' << values[i + 2];
        // std::cout << ' ' << std::bit_cast<int32_t>( values[i + 3] )
        //           << std::endl;
    }
}

MotorControlServiceCore::MotorControlServiceCore() : UniversalServerCore("MotorControlServiceCore") {}

MotorControlServiceCore::MotorControlServiceCore(const std::shared_ptr<ModuleFT232RL> &m_)
    : UniversalServerCore("MotorControlServiceCore")
    , module_(m_)
{}

MotorControlServiceCore::~MotorControlServiceCore() {}

void MotorControlServiceCore::Init() {}

void MotorControlServiceCore::Process(const int id, const std::string &name, const std::string &msg)
{
    (void)id;
    (void)name;
    pkg::Message message_in;
    try
    {
        message_in = deserialize<pkg::Message>(msg);
    }
    catch (...)
    {
        std::cout << std::format(">>>> Баг в сообщении: {}", msg) << std::endl;
        // pkg::ImOkay message_err;
        // message_err.status = INT_MAX;
        // writeToSock( id, serialize( message_err ) );
        return;
    }

    mcs::MotorsGroupSettings message;
    try
    {
        message = deserialize<mcs::MotorsGroupSettings>(message_in.text);
    }
    catch (...)
    {
        std::cout << std::format("===> Баг в сообщении: {}", message_in.text) << std::endl;
        // pkg::ImOkay message_err;
        // message_err.status = INT_MAX;
        // writeToSock( id, serialize( message_err ) );
        return;
    }

    std::cout << std::format(
        "configureAllEngines: {}, startSimultaneously: {}, "
        "startImmediately: {}",
        message.configureAllEngines.size(),
        message.startSimultaneously.size(),
        message.startImmediately.size())
              << std::endl;

    std::vector<uint32_t> commandsMessageToBoard(MOTOR_COUNT * MOTOR_CELLS_COUNT, 0);

    setCAE(commandsMessageToBoard, message);
    setSS(commandsMessageToBoard, message);
    setSI(commandsMessageToBoard, message);
    print(commandsMessageToBoard);
    module_->writeData(commandsMessageToBoard);

    // std::vector<uint32_t> m;
    // m.resize( 64 );
    // module_->readData( m );
    // print( m );
    //     auto result_ = module_->read<uint32_t>();
    //     print( result_ );
}

void MotorControlServiceCore::Launch() {}

void MotorControlServiceCore::Stop() {}

void MotorControlServiceCore::checkMotorNum(const uint32_t num, const std::string &name)
{
    if (num > 16 or num < 1)
        throw std::invalid_argument(std::format(
            "[MotorControlServiceCore]({}): motor "
            "num = {}, must be > 0 and < 17",
            name,
            num));
}

void MotorControlServiceCore::setMotor(
    std::vector<uint32_t> &values,
    const size_t i,
    const uint32_t settings,
    const mcs::MotorSettings &motor_)
{
    values[(i + 0)] = settings;
    values[(i + 1)] = motor_.acceleration;
    values[(i + 2)] = motor_.maxSpeed;
    values[(i + 3)] = motor_.step;
}

void MotorControlServiceCore::setMotors(
    std::vector<uint32_t> &values,
    const size_t i,
    const uint32_t settings,
    const mcs::MotorsSettings &motors_)
{
    values[(i + 0)] = settings;
    values[(i + 1)] = motors_.acceleration;
    values[(i + 2)] = motors_.maxSpeed;
    values[(i + 3)] = motors_.step;
}

/*
 * @brief setCAE - установка configureAllEngines
 * Возможна коллизия, при которой произайдет перезапись одного
 * движка поверх другого, это нормально
 * TODO(khosta77) : прописать в КД, коллизию выше
 * */
void MotorControlServiceCore::setCAE(std::vector<uint32_t> &values, const mcs::MotorsGroupSettings &msg)
{
    if (!msg.configureAllEngines.empty())
    {
        for (const auto &group : msg.configureAllEngines)
        {
            uint32_t settings = 0b00000000000000001000000000000000;
            for (const auto &m : group.motors)
            {
                checkMotorNum(m, "setConfigureAllEngines");
                settings |= (1U << (32 - static_cast<uint32_t>(m)));
            }

            for (const auto &g_ : group.motors)
                setMotors(values, ((g_ - 1) * MOTOR_CELLS_COUNT), settings, group);
        }
    }
}

/*
 * @brief setSS - установка startSimultaneously
 * Возможна коллизия, при которой произайдет перезапись одного
 * движка поверх другого, это нормально
 * TODO(khosta77) : прописать в КД, коллизию выше
 * */
void MotorControlServiceCore::setSS(std::vector<uint32_t> &values, const mcs::MotorsGroupSettings &msg)
{
    if (!msg.startSimultaneously.empty())
    {
        auto groups = msg.startSimultaneously;
        uint32_t settings = 0b00000000000000000100000000000000;
        for (const auto &group : groups)
        {
            checkMotorNum(group.motor, "setStartSimultaneously");
            settings |= (1U << (32 - static_cast<uint32_t>(group.motor)));
        }

        for (const auto &g_ : groups)
            setMotor(values, ((g_.motor - 1) * MOTOR_CELLS_COUNT), settings, g_);
    }
}

/*
 * @brief setSI - установка startImmediately
 * Возможна коллизия, при которой произайдет перезапись одного
 * движка поверх другого, это нормально
 * TODO(khosta77) : прописать в КД, коллизию выше
 * */
void MotorControlServiceCore::setSI(std::vector<uint32_t> &values, const mcs::MotorsGroupSettings &msg)
{
    if (!msg.startImmediately.empty())
    {
        for (const auto &g_ : msg.startImmediately)
        {
            checkMotorNum(g_.motor, "setStartImmediately");
            setMotor(
                values,
                ((g_.motor - 1) * MOTOR_CELLS_COUNT),
                ((1U << (32 - static_cast<uint32_t>(g_.motor))) | 0x2000),
                g_);
        }
    }
}
