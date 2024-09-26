cmake_minimum_required(VERSION 3.13...3.27)

set(NAME software_download_lib)
add_library(${NAME}
    src/software_download.cpp
    src/hal/pico_interface_impl.cpp
)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include_common)

target_link_libraries(${NAME}
    pico_flash
    pico_stdlib                        # for core pico functionality
    pico_mbedtls
    hardware_exception
    hardware_watchdog
)

target_compile_definitions(${NAME} PRIVATE -DBOOTLOADER_BUILD)
