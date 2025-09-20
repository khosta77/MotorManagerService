cmake_minimum_required(VERSION 3.28)
project(UniversalServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)

if(APPLE)
    include_directories(/opt/homebrew/opt/boost/include)
else()
    find_package(Boost REQUIRED)
    find_package(GTest REQUIRED)
endif()

add_subdirectory(source)

if(BUILD_TESTS)
    include(CTest)
    enable_testing()

    add_subdirectory(test)

    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --progress --verbose
        VERBATIM
    )
    add_dependencies(run_tests all_unit_tests)
    #add_custom_target(test_1
    #    COMMAND
    #        ${CMAKE_CTEST_COMMAND} -R "Exceptions_tests" --output-on-failure
    #    VERBATIM
    #)
    #add_dependencies(test_1 exceptions_tests)
endif()
