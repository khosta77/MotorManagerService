set(TEST_NAME mms_module_rs232_ft232rl_unit_tests)
file(GLOB EXCEPTIONS_TEST_SOURCES "*.cpp")

add_executable(${TEST_NAME} ${EXCEPTIONS_TEST_SOURCES})
target_include_directories(${TEST_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include/service_host
        ${CMAKE_SOURCE_DIR}/include/module_rs232
        ${CMAKE_SOURCE_DIR}/include/core
        ${FTD2XX_LIB}
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        GTest::gmock
        GTest::gtest_main
        module_rs232
        ${FTD2XX_LIB}
)

set_target_properties(${TEST_NAME}
    PROPERTIES
        INSTALL_RPATH "@loader_path"
        BUILD_WITH_INSTALL_RPATH TRUE
)

add_custom_command(
    TARGET ${TEST_NAME} POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/driver/libftd2xx.dylib
        ${CMAKE_BINARY_DIR}/test/unit/module_rs232/ft232rl/libftd2xx.dylib
    COMMENT "copy libftd2xx.dylib to build/test/unit/module_rs232/ft232rl/"
)

target_compile_options(${TEST_NAME} PUBLIC ${COVERAGE_FLAGS})

add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
set(TEST_TARGET_NAME ${TEST_NAME} PARENT_SCOPE)
