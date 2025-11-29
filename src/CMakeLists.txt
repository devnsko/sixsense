# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

# REQUIRES_PRIV esp_driver_gpio

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/main.cpp)

idf_component_register(SRCS ${app_sources}
                    REQUIRES esp_driver_dac)