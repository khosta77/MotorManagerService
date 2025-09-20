add_subdirectory(exceptions)
add_subdirectory(network_serializer)

add_custom_target(service_host_tests)
add_dependencies(service_host_tests ${TEST_TARGET_NAME})

set(SERVICE_HOST_TEST_TARGET ${TEST_TARGET_NAME} PARENT_SCOPE)
