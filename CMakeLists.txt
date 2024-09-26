cmake_minimum_required(VERSION 3.13...3.27)

set(PICO_BOARD pico_w) # Set the board used
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
else ()
    # Pull in Pico SDK (must be before project)
    include(pico_sdk_import.cmake)
endif()

project(pico_bootloader)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(unittests.cmake)
else ()
    set(NAME pico_bootloader)
    add_executable(${NAME}
        src/main.cpp
        software_download/src/bootloader.cpp
    )

    # initialize the Raspberry Pi Pico SDK
    pico_sdk_init()

    pico_enable_stdio_usb(${NAME} 0)
    pico_enable_stdio_uart(${NAME} 1)

    # create map/bin/hex file etc.
    pico_add_extra_outputs(${NAME})

    pico_set_linker_script(${NAME} ${CMAKE_CURRENT_SOURCE_DIR}/linker_common/bootloader.ld)

    include_directories(linker_common)
    include_directories(include_common)
    include_directories(software_download/src)

    add_subdirectory(software_download)
    add_subdirectory(example_app)

    target_link_libraries(${NAME}
        pico_stdlib                        # for core pico functionality
        cmsis_core
        pico_base
        software_download_lib
    )
    target_link_options(${NAME} PRIVATE
        -L${CMAKE_CURRENT_SOURCE_DIR}/linker_common
    )
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
)
