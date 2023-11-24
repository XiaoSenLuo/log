cmake_minimum_required(VERSION 3.19.0)
add_library(log STATIC)
target_include_directories(log PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_sources(log PRIVATE
        "log.c"
)

target_link_libraries(log PRIVATE FreeRTOS printf)
