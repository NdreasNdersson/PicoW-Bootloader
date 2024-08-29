cmake_minimum_required(VERSION 3.13...3.27)

set(NAME PICO_BOOTLOADER)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w) # Set the board used

# initialize pico-sdk from submodule
# note: this must happen before project()
include(../external/pico-sdk/pico_sdk_init.cmake)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${NAME}
    src/bootloader.c
)

target_link_libraries(${NAME}
    pico_stdlib                        # for core pico functionality
    cmsis_core
)

pico_enable_stdio_usb(${NAME} 0)
pico_enable_stdio_uart(${NAME} 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

pico_set_linker_script(${NAME} ${CMAKE_SOURCE_DIR}/memmap_custom.ld)

