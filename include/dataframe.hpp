#ifndef DATAFRAME_HPP_
#define DATAFRAME_HPP_

/*
{
    "mode": "synchronous" | "asynchronous",
    "motors" : [
        {
            "номер" : 1-10,
            "ускорен" : ...,
            "максскор" : ...,
            "шаги" : +-...,
        }
    ]
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
