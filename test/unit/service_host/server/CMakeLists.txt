set(TEST_NAME mms_service_host_server_unit_tests)
file(GLOB EXCEPTIONS_TEST_SOURCES "*.cpp")

add_executable(${TEST_NAME} ${EXCEPTIONS_TEST_SOURCES})
target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/service_host)
target_link_libraries(${TEST_NAME}
    GTest::gmock
    GTest::gtest_main
    service_host
)

add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
set(TEST_TARGET_NAME ${TEST_NAME} PARENT_SCOPE)
