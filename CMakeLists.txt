cmake_minimum_required(VERSION 3.13...3.27)

set(NAME PICO_BOOTLOADER)
set(PICO_BOARD pico_w) # Set the board used
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(external/pico-sdk/pico_sdk_init.cmake)

project(${NAME})
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

include_directories(linker_common)
include_directories(include_common)
add_subdirectory(bootloader)
add_subdirectory(example_app)

