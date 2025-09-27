add_subdirectory(ft232rl)

set(ALL_MODULE_RS232_TEST_TARGETS
    mms_module_rs232_ft232rl_unit_tests
)

add_custom_target(module_rs232_tests)
add_dependencies(module_rs232_tests ${ALL_MODULE_RS232_TEST_TARGETS})
