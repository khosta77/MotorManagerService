add_subdirectory(exceptions)
add_subdirectory(network_serializer)
add_subdirectory(server)
add_subdirectory(utils)

set(ALL_SERVICE_HOST_TEST_TARGETS
    mms_service_host_exceptions_unit_tests
    mms_service_host_network_serializer_unit_tests
    mms_service_host_server_unit_tests
    mms_service_host_utils_unit_tests
)

add_custom_target(service_host_tests)
add_dependencies(service_host_tests ${ALL_SERVICE_HOST_TEST_TARGETS})
