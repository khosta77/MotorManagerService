#ifndef DATAFRAME_HPP_
#define DATAFRAME_HPP_

/*
{
  "configureAllEngines": [
    {
      "motors": [1, 2, 3],
      "acceleration" : 8000,
      "maxSpeed" : 16000,
      "step" : 800
    }
  ],
  "startSimultaneously": [
      {
        "motor" : 4,
        "acceleration" : "uint32_t",
        "maxSpeed" : "uint32_t",
        "step" : "int32_t"
      }
    ],
  "startImmediately": [
      {
        "motor" : 4,
        "acceleration" : "uint32_t",
        "maxSpeed" : "uint32_t",
        "step" : "int32_t"
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
    ( mcs ), MotorSettings,
    ( int, motor )
    ( uint32_t, acceleration )
    ( uint32_t, maxSpeed )
    ( int32_t, step )
)

BOOST_FUSION_DEFINE_STRUCT(
    ( mcs ), MotorsSettings,
    ( std::vector<int>, motors )
    ( uint32_t, acceleration )
    ( uint32_t, maxSpeed )( int32_t, step )
)

BOOST_FUSION_DEFINE_STRUCT(
    ( mcs ), MotorsGroupSettings,
    ( std::vector<mcs::MotorsSettings>, configureAllEngines )
    ( std::vector<mcs::MotorSettings>, startSimultaneously )
    ( std::vector<mcs::MotorSettings>, startImmediately )
)
// clang-format on

#endif // DATAFRAME_HPP_
