# Библиотека service_host
add_library(service_host
    STATIC
        service_host/exceptions.cpp
        service_host/network_serializer.cpp
        service_host/server.cpp
        service_host/socket.cpp
        service_host/utils.cpp
)

target_include_directories(service_host
    PUBLIC
        ${CMAKE_SOURCE_DIR}/json/single_include
        ${CMAKE_SOURCE_DIR}/include/service_host
)

# Библиотека module_rs232
add_library(module_ft232rl
    STATIC
        module_rs232/ft232rl.cpp
)

target_include_directories(module_ft232rl
    PUBLIC
        ${FTD2XX_LIB}
)

target_link_libraries(module_ft232rl
    PRIVATE
        service_host
        ${FTD2XX_LIB}
)

target_include_directories(module_ft232rl
    PUBLIC
        ${CMAKE_SOURCE_DIR}/json/single_include
        ${CMAKE_SOURCE_DIR}/include/service_host
        ${CMAKE_SOURCE_DIR}/driver/ftd2xxlib
        ${CMAKE_SOURCE_DIR}/include/module_rs232
)

set_target_properties(module_ft232rl PROPERTIES
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH TRUE
)

add_custom_command(
    TARGET module_ft232rl POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/driver/libftd2xx.dylib
        ${CMAKE_BINARY_DIR}/libftd2xx.dylib
    COMMENT "copy libftd2xx.dylib to build/"
)

add_executable(ft232rl_main module_rs232/ft232rl_main.cpp)
target_link_libraries(ft232rl_main module_ft232rl)

# Библиотека core
add_library(user_core
    STATIC
        core/user_core.cpp
)

target_include_directories(user_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/core
        ${CMAKE_SOURCE_DIR}/json/single_include
        ${CMAKE_SOURCE_DIR}/json/single_include
        ${CMAKE_SOURCE_DIR}/include/service_host
        ${CMAKE_SOURCE_DIR}/driver/ftd2xxlib
        ${CMAKE_SOURCE_DIR}/include/module_rs232
)

target_link_libraries(user_core
    PUBLIC
        service_host
        module_ft232rl
)

# Основное приложение
#add_executable(universal_server core/main.cpp)
#target_link_libraries(universal_server user_core module_rs232 service_host)
