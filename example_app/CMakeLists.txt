cmake_minimum_required(VERSION 3.13...3.27)

set(NAME ${NAME}_EXAMPLE_APP)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${NAME}
    src/picow_blink.c
)

target_link_libraries(${NAME}
    pico_stdlib              # for core functionality
    pico_cyw43_arch_none     # we need Wifi to access the GPIO, but we don't need anything else
)

target_link_options(${NAME} PRIVATE
    "-L${CMAKE_SOURCE_DIR}/linker_common"
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

pico_set_linker_script(${NAME} ${CMAKE_SOURCE_DIR}/linker_common/app.ld)
