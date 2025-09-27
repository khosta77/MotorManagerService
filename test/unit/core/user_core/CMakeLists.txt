set(TEST_NAME mms_core_user_core_unit_tests)
file(GLOB EXCEPTIONS_TEST_SOURCES "*.cpp")

add_executable(${TEST_NAME} ${EXCEPTIONS_TEST_SOURCES})
target_include_directories(${TEST_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include/service_host
        ${CMAKE_SOURCE_DIR}/include/module_rs232
        ${CMAKE_SOURCE_DIR}/include/core
)
target_link_libraries(${TEST_NAME}
    PRIVATE
        GTest::gmock
        GTest::gtest_main
        user_core
        -fprofile-generate
)
target_compile_options(${TEST_NAME} PUBLIC ${COVERAGE_FLAGS})

add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
set(TEST_TARGET_NAME ${TEST_NAME} PARENT_SCOPE)
