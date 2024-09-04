cmake_minimum_required(VERSION 3.13...3.27)

set(NAME ${NAME}_LIB)
add_library(${NAME}
    src/bootloader_lib.cpp
)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include_common)

target_link_libraries(${NAME}
    pico_flash
    pico_stdlib                        # for core pico functionality
    pico_mbedtls
    hardware_exception
)
