cmake_minimum_required(VERSION 3.28)
project(MotorManagerServiceTest)

if(ENABLE_COVERAGE)
    set(COVERAGE_FLAGS --coverage -g -O0)
else()
    set(COVERAGE_FLAGS "")
endif()

if(APPLE)
    set(GTEST_ROOT "/opt/homebrew/opt/googletest")
    include_directories(${GTEST_ROOT}/include)
    include_directories(/opt/homebrew/opt/boost/include)
endif()

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

include(GoogleTest)

add_subdirectory(unit/service_host)
add_subdirectory(unit/module_rs232)
add_subdirectory(unit/core)

add_custom_target(all_unit_tests)
add_dependencies(all_unit_tests
    service_host_tests
    module_rs232_tests
    core_tests
)
