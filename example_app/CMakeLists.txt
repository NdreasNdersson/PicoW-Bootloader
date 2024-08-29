cmake_minimum_required(VERSION 3.13...3.27)

set(NAME PICO_BOOTLOADER_EXAMPLE_APP)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w) # Set the board used

# initialize pico-sdk from submodule
# note: this must happen before project()
include(../external/pico-sdk/pico_sdk_init.cmake)

project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${NAME}
    src/picow_blink.c
)

target_link_libraries(${NAME}
    pico_stdlib              # for core functionality
    pico_cyw43_arch_none     # we need Wifi to access the GPIO, but we don't need anything else
)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

pico_set_linker_script(${NAME} ${CMAKE_SOURCE_DIR}/memmap_custom.ld)

