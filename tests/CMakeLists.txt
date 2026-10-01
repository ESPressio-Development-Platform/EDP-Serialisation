cmake_minimum_required(VERSION 3.16)
project(EDP_Serialisation_Tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

foreach(required_dir
    EDP_SYSTEM_SOURCE_DIR
    EDP_BOUNDED_TYPES_SOURCE_DIR
    EDP_PLATFORM_SOURCE_DIR
    EDP_MEMORY_SOURCE_DIR
    EDP_BOUNDED_TOPOLOGY_SOURCE_DIR
    EDP_PLATFORM_PORTABLE_SOURCE_DIR
)
    if(NOT DEFINED ${required_dir})
        message(FATAL_ERROR "${required_dir} must point at the corresponding ESPressio checkout")
    endif()
endforeach()

add_executable(EDP_Serialisation_HostTests host/main.cpp)
target_include_directories(EDP_Serialisation_HostTests PRIVATE
    ../src
    ${EDP_SYSTEM_SOURCE_DIR}/src
    ${EDP_BOUNDED_TYPES_SOURCE_DIR}/src
    ${EDP_PLATFORM_SOURCE_DIR}/src
    ${EDP_MEMORY_SOURCE_DIR}/src
    ${EDP_BOUNDED_TOPOLOGY_SOURCE_DIR}/src
    ${EDP_PLATFORM_PORTABLE_SOURCE_DIR}/src
)
target_compile_options(EDP_Serialisation_HostTests PRIVATE -Wall -Wextra -Wpedantic -Werror)
