cmake_minimum_required(VERSION 3.28)
project(MockMCU)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройки для macOS
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES arm64)
endif()

# Поиск библиотек
find_package(Threads REQUIRED)

# Включаемые директории
include_directories(
    ${CMAKE_SOURCE_DIR}/../include/module_rs232
    ${CMAKE_SOURCE_DIR}/../include/service_host
    ${CMAKE_SOURCE_DIR}/../driver/ftd2xxlib
    ${CMAKE_SOURCE_DIR}/../json/single_include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Создание исполняемого файла mock-MCU
add_executable(mock-mcu
    src/main.cpp
    src/mock_mcu.cpp
    src/protocol_handler.cpp
)

# Линковка библиотек
target_link_libraries(mock-mcu
    PRIVATE
        ${CMAKE_SOURCE_DIR}/../build/source/libmodule_rs232.a
        ${CMAKE_SOURCE_DIR}/../build/source/libservice_host.a
        ${CMAKE_SOURCE_DIR}/../driver/libftd2xx.dylib
        Threads::Threads
)

# Настройки для macOS
set_target_properties(mock-mcu
    PROPERTIES
        INSTALL_RPATH "@loader_path"
        BUILD_WITH_INSTALL_RPATH TRUE
)

# Копирование библиотеки FTDI
add_custom_command(
    TARGET mock-mcu POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/../driver/libftd2xx.dylib
        ${CMAKE_BINARY_DIR}/libftd2xx.dylib
    COMMENT "copy libftd2xx.dylib to build/"
)

# Компиляционные флаги
target_compile_options(mock-mcu PRIVATE -Wall -Wextra -O2 -fno-profile-arcs -fno-test-coverage)
target_link_options(mock-mcu PRIVATE -fno-profile-arcs -fno-test-coverage)
