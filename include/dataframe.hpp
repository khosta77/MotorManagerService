#ifndef DATAFRAME_HPP_
#define DATAFRAME_HPP_

/*
Примеры JSON для каждой структуры:

1) mms::MotorSettings
{
    "number": 1,
    "acceleration": 2000,
    "maxSpeed": 5000,
    "step": 100
}

2) mms::MotorsSettings
{
    "mode": "synchronous",
    "motors": [
        {"number": 1, "acceleration": 2000, "maxSpeed": 5000, "step": 100},
        {"number": 2, "acceleration": 1500, "maxSpeed": 4500, "step": -50}
    ]
}

3) mms::Version
{
    "version": 1.2,
    "name": "Squid"
}

4) mms::Device
{
    "deviceId": 0
}

5) mms::ListConnect
{
    "listConnect": ["COM0", "COM1"]
}

6) mms::Manager
{
    "command": "version",
    "message": ""
}
*/

// clang-format off
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/struct.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/type_index.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/type_index.hpp>

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
    (std::vector<mms::MotorSettings>, motors)
)

BOOST_FUSION_DEFINE_STRUCT(
    (mms), Version,
    (float, version)
    (std::string, name)
)

BOOST_FUSION_DEFINE_STRUCT(
    (mms), Device,
    (int, deviceId)
)

BOOST_FUSION_DEFINE_STRUCT(
    (mms), ListConnect,
    (std::vector<std::string>, listConnect)
)

BOOST_FUSION_DEFINE_STRUCT(
    (mms), Manager,
    (std::string, command)  // command
    (std::string, message)  // сообщение для команды
)
// command: version()
//          moving(MotorsSettings)
//          reconnect(id)
//          disconnect()
//          listconnect()
// clang-format on

#endif // DATAFRAME_HPP_
