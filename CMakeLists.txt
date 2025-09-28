cmake_minimum_required(VERSION 3.28)
project(MotorManagerService LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FTD2XX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/driver/ftd2xxlib")
set(FTD2XX_LIB "${CMAKE_CURRENT_SOURCE_DIR}/driver/libftd2xx.dylib")
set(FTD2XX_BLB "${CMAKE_CURRENT_SOURCE_DIR}/build/libftd2xx.dylib")

set(BUILD_TESTS ON)
set(ENABLE_COVERAGE ON)

if(ENABLE_COVERAGE)
    add_compile_options(--coverage)
    add_link_options(--coverage)

    message(STATUS "coverage (ON)")
else()
    message(STATUS "coverage (OFF)")
endif()

if(APPLE)
    include_directories(/opt/homebrew/opt/boost/include)
else()
    find_package(Boost REQUIRED)
    find_package(GTest REQUIRED)
endif()

if(NOT EXISTS "${FTD2XX_LIB}")
    message(FATAL_ERROR "libftd2xx.dylib not found at: ${FTD2XX_LIB}")
else()
    message(STATUS "Found libftd2xx: ${FTD2XX_LIB}")
endif()

add_compile_options(-Werror)

add_subdirectory(source)

if(BUILD_TESTS)
    include(CTest)
    enable_testing()

    add_subdirectory(test)

    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        VERBATIM
    )
    add_dependencies(run_tests all_unit_tests)
endif()

if(ENABLE_COVERAGE)
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)

    if(LCOV AND GENHTML)
        add_custom_target(coverage
            COMMAND
                ${LCOV} --directory . --zerocounters > /dev/null 2>&1 || true
            COMMAND
                ${CMAKE_CTEST_COMMAND} --output-on-failure || true
            COMMAND
                ${LCOV}
                --ignore-errors inconsistent,inconsistent,inconsistent,format,unsupported,category,corrupt,mismatch
                --capture
                --directory .
                --output-file coverage.info > /dev/null 2>&1
            COMMAND
                ${LCOV}
                --ignore-errors format,unsupported,inconsistent,corrupt,unused
                --remove coverage.info
                "*/opt/*"
                "*/v1/*"
                "*/Library/Developer/CommandLineTools/SDKs/*"
                "*/test/*"
                "*/tests/*"
                "*/tests_*/*"
                "*/json/*"
                "*/usr/*"
                --output-file coverage.info.filtered > /dev/null 2>&1
            COMMAND
                mv coverage.info.filtered coverage.info || cp coverage.info coverage.info.filtered
            COMMAND
                ${GENHTML}
                --ignore-errors category,category,inconsistent,inconsistent,unsupported,category,inconsistent,corrupt
                coverage.info
                --output-directory coverage_report
            COMMAND
                ${LCOV} --summary coverage.info 2>/dev/null | grep lines || true
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating coverage report..."
        )
    else()
        message(WARNING "lcov or genhtml not found! Coverage targets disabled.")
    endif()
endif()
