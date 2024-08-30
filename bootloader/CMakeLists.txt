cmake_minimum_required(VERSION 3.13...3.27)

add_executable(${NAME}
    src/bootloader.cpp
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${NAME}
    pico_stdlib                        # for core pico functionality
    cmsis_core
    pico_mbedtls
)

target_link_options(${NAME} PRIVATE
    -L${CMAKE_SOURCE_DIR}/linker_common
)

pico_enable_stdio_usb(${NAME} 0)
pico_enable_stdio_uart(${NAME} 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

pico_set_linker_script(${NAME} ${CMAKE_SOURCE_DIR}/linker_common/bootloader.ld)
