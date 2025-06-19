cmake_minimum_required(VERSION 3.28)
project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
if(ENABLE_COVERAGE)
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

add_compile_options(-Wall)
add_compile_options(-Werror)
add_compile_options(-Wextra)
add_compile_options(-Wsign-compare)
add_compile_options(-Wno-unknown-attributes)

if(APPLE)
    include_directories(/opt/homebrew/opt/boost/include)
else()
    find_package(Boost REQUIRED)
    find_package(GTest REQUIRED)
endif()

enable_testing()

# Подключаем подпроекты ПЕРВЫМИ
add_subdirectory(source/UniversalServerUnits/tests_Exceptions)
add_subdirectory(source/UniversalServerUnits/tests_utils)
add_subdirectory(source/UniversalServerUnits/tests_UniversalServerUnits_json)
add_subdirectory(source/UniversalServerUnits/tests_UniversalServerUnits_split)
add_subdirectory(source/UniversalServerUnits/tests_UniversalServerUnits_readFromSocket)
add_subdirectory(source/UniversalServerUnits/tests_UniversalServerUnits_writeToSocket)
add_subdirectory(source/UniversalServerUnits/tests_UniversalServerUnits_server)
#add_subdirectory(source/MotorControlService/tests)

# Теперь можно ссылаться на цели
add_custom_target(test_1 COMMAND ${CMAKE_CTEST_COMMAND} -R "Exceptions_tests" --output-on-failure VERBATIM)
add_dependencies(test_1 Exceptions_tests)

add_custom_target(test_2 COMMAND ${CMAKE_CTEST_COMMAND} -R "utils_tests" --output-on-failure VERBATIM)
add_dependencies(test_2 utils_tests)

add_custom_target(test_3 COMMAND ${CMAKE_CTEST_COMMAND} -R "UniversalServerUnits_json_tests" --output-on-failure VERBATIM)
add_dependencies(test_3 UniversalServerUnits_json_tests)

add_custom_target(test_4 COMMAND ${CMAKE_CTEST_COMMAND} -R "UniversalServerUnits_split_tests" --output-on-failure VERBATIM)
add_dependencies(test_4 UniversalServerUnits_split_tests)

add_custom_target(test_5 COMMAND ${CMAKE_CTEST_COMMAND} -R "UniversalServerUnits_readFromSocket_tests" --output-on-failure VERBATIM)
add_dependencies(test_5 UniversalServerUnits_readFromSocket_tests)

add_custom_target(test_6 COMMAND ${CMAKE_CTEST_COMMAND} -R "UniversalServerUnits_writeToSocket_tests" --output-on-failure VERBATIM)
add_dependencies(test_6 UniversalServerUnits_writeToSocket_tests)

add_custom_target(test_7 COMMAND ${CMAKE_CTEST_COMMAND} -R "UniversalServerUnits_server_tests" --output-on-failure VERBATIM)
add_dependencies(test_7 UniversalServerUnits_server_tests)

set(FTD2XX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/driver/ftd2xxlib")
set(FTD2XX_LIB "${CMAKE_CURRENT_SOURCE_DIR}/driver/libftd2xx.dylib")
set(FTD2XX_BLB "${CMAKE_CURRENT_SOURCE_DIR}/build/libftd2xx.dylib")


# Проверяем, существует ли библиотека
if(NOT EXISTS "${FTD2XX_LIB}")
    message(FATAL_ERROR "libftd2xx.dylib not found at: ${FTD2XX_LIB}")
else()
    message(STATUS "Found libftd2xx: ${FTD2XX_LIB}")
endif()

# Сервер преобразование
add_executable(MotorControlService.out
    source/MotorControlService/main.cpp
    source/MotorControlService/MotorControlServiceCore.cpp
    source/UniversalServerUnits/utils.cpp
    source/ModuleFT232RL/exceptions.cpp
    source/ModuleFT232RL/ModuleFT232RL.cpp
    source/UniversalServerUnits/Exceptions.cpp
    source/UniversalServerUnits/SocketWrapper.cpp
    source/UniversalServerUnits/UniversalServer.cpp
    source/UniversalServerUnits/UniversalServerUnits.cpp)

target_include_directories(MotorControlService.out PUBLIC ${FTD2XX_LIB})
target_link_libraries(MotorControlService.out PRIVATE ${FTD2XX_LIB})

# Указываем, что библиотека должна искаться рядом с исполняемым файлом
set_target_properties(MotorControlService.out PROPERTIES
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH TRUE
)

add_custom_command(
    TARGET MotorControlService.out POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/driver/libftd2xx.dylib
        ${CMAKE_BINARY_DIR}/libftd2xx.dylib
    COMMENT "copy libftd2xx.dylib to build/"
)

target_include_directories(MotorControlService.out PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/driver/ftd2xxlib
    ${CMAKE_CURRENT_SOURCE_DIR}/source/UniversalServerUnits
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ModuleFT232RL
    ${CMAKE_CURRENT_SOURCE_DIR}/source/MotorControlService)
add_dependencies(MotorControlService.out test_1 test_2 test_3 test_4 test_5 test_6 test_7)

# покрытие
if(ENABLE_COVERAGE)
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)

    if(LCOV AND GENHTML)
        add_custom_target(coverage
            # Собираем данные
            COMMAND ${LCOV}
                --ignore-errors inconsistent,inconsistent,inconsistent,format,unsupported,category,corrupt,mismatch
                --capture
                --directory .
                --output-file coverage.info > /dev/null 2>&1
            # Фильтруем результаты
            COMMAND ${LCOV}
                --ignore-errors format,unsupported,inconsistent,corrupt,unused,unused
                --remove coverage.info
                "*/opt/*"
                "*/v1/*"
                "*/Library/Developer/CommandLineTools/SDKs/*"
                "*/tests/*"
                "*/tests_*/*"
                "*/json/*"
                --output-file coverage.info > /dev/null 2>&1
            # Генерируем HTML
            COMMAND ${GENHTML}
                --ignore-errors category,category,inconsistent,inconsistent,unsupported,category,inconsistent,corrupt
                coverage.info
                --output-directory coverage_report
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS
               utils_tests
               Exceptions_tests
               UniversalServerUnits_json_tests
               UniversalServerUnits_split_tests
               UniversalServerUnits_readFromSocket_tests
               UniversalServerUnits_writeToSocket_tests
               UniversalServerUnits_server_tests
            COMMENT "Generating coverage report..."
        )
    else()
        message(WARNING "lcov or genhtml not found! Coverage targets disabled.")
    endif()
endif()
