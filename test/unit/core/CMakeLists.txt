add_subdirectory(user_core)

set(ALL_USER_CORE_TEST_TARGETS
    mms_core_user_core_unit_tests
)

add_custom_target(core_tests)
add_dependencies(core_tests ${ALL_USER_CORE_TEST_TARGETS})
