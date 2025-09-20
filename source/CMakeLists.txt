# Библиотека service_host
add_library(service_host STATIC
    service_host/exceptions.cpp
    service_host/network_serializer.cpp
    service_host/server.cpp
    service_host/socket.cpp
    service_host/utils.cpp
)

target_include_directories(service_host PUBLIC
    ${CMAKE_SOURCE_DIR}/json/single_include
    ${CMAKE_SOURCE_DIR}/include/service_host
)

# Библиотека core
#add_library(user_core STATIC
#    core/user_core.cpp
#)

#target_include_directories(user_core PUBLIC
#    ${CMAKE_SOURCE_DIR}/include/core
#)

#target_link_libraries(user_core PUBLIC service_host)

# Библиотека module_rs232
#add_library(module_rs232 STATIC
#    module_rs232/ft232rl.cpp
#)

#target_include_directories(module_rs232 PUBLIC
#    ${CMAKE_SOURCE_DIR}/include/module_rs232
#)#

# Основное приложение
#add_executable(universal_server core/main.cpp)
#target_link_libraries(universal_server user_core module_rs232 service_host)
