cmake_minimum_required(VERSION 3.16)
project(EDP_Serialisation_Json_Numeric_Round_Trip LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

get_filename_component(
    EDP_SERIALISATION_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../../.."
    ABSOLUTE
)
get_filename_component(
    EDP_WORKSPACE_DIR
    "${EDP_SERIALISATION_SOURCE_DIR}/.."
    ABSOLUTE
)

if(NOT DEFINED EDP_SYSTEM_SOURCE_DIR)
    set(EDP_SYSTEM_SOURCE_DIR "${EDP_WORKSPACE_DIR}/EDP-System")
endif()
if(NOT DEFINED EDP_BOUNDED_TYPES_SOURCE_DIR)
    set(EDP_BOUNDED_TYPES_SOURCE_DIR "${EDP_WORKSPACE_DIR}/EDP-BoundedTypes")
endif()
if(NOT DEFINED EDP_PLATFORM_SOURCE_DIR)
    set(EDP_PLATFORM_SOURCE_DIR "${EDP_WORKSPACE_DIR}/EDP-Platform")
endif()
if(NOT DEFINED EDP_MEMORY_SOURCE_DIR)
    set(EDP_MEMORY_SOURCE_DIR "${EDP_WORKSPACE_DIR}/EDP-Memory")
endif()
if(NOT DEFINED EDP_BOUNDED_TOPOLOGY_SOURCE_DIR)
    set(EDP_BOUNDED_TOPOLOGY_SOURCE_DIR "${EDP_WORKSPACE_DIR}/EDP-BoundedTopology")
endif()
if(NOT DEFINED EDP_PLATFORM_PORTABLE_SOURCE_DIR)
    set(EDP_PLATFORM_PORTABLE_SOURCE_DIR "${EDP_WORKSPACE_DIR}/EDP-Platform-Portable")
endif()
if(NOT DEFINED EDP_LOCALISATION_SOURCE_DIR)
    set(EDP_LOCALISATION_SOURCE_DIR "${EDP_WORKSPACE_DIR}/EDP-Localisation")
endif()

foreach(required_dir
    EDP_SYSTEM_SOURCE_DIR
    EDP_BOUNDED_TYPES_SOURCE_DIR
    EDP_PLATFORM_SOURCE_DIR
    EDP_MEMORY_SOURCE_DIR
    EDP_BOUNDED_TOPOLOGY_SOURCE_DIR
    EDP_PLATFORM_PORTABLE_SOURCE_DIR
    EDP_LOCALISATION_SOURCE_DIR
)
    if(NOT EXISTS "${${required_dir}}/src")
        message(FATAL_ERROR "${required_dir} must point at the corresponding ESPressio checkout")
    endif()
endforeach()

add_executable(
    EDP_Serialisation_JsonNumericRoundTrip
    main.cpp
)
target_include_directories(
    EDP_Serialisation_JsonNumericRoundTrip
    PRIVATE
        ${EDP_SERIALISATION_SOURCE_DIR}/src
        ${EDP_SYSTEM_SOURCE_DIR}/src
        ${EDP_BOUNDED_TYPES_SOURCE_DIR}/src
        ${EDP_PLATFORM_SOURCE_DIR}/src
        ${EDP_MEMORY_SOURCE_DIR}/src
        ${EDP_BOUNDED_TOPOLOGY_SOURCE_DIR}/src
        ${EDP_PLATFORM_PORTABLE_SOURCE_DIR}/src
        ${EDP_LOCALISATION_SOURCE_DIR}/src
)
target_compile_options(
    EDP_Serialisation_JsonNumericRoundTrip
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
)
