cmake_minimum_required(VERSION 3.13...3.27)

enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/stubs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

package_add_test(BootloaderTest
    ${CMAKE_CURRENT_SOURCE_DIR}/bootloader_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/linker_definitions_stub.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/bootloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/software_download.cpp
)
target_compile_definitions(BootloaderTest PRIVATE -DBOOTLOADER_BUILD)
target_compile_definitions(BootloaderTest PRIVATE -DBOOTLOADER_TEST)

package_add_test(SoftwareDownloadTest
    ${CMAKE_CURRENT_SOURCE_DIR}/software_download_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/linker_definitions_stub.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/software_download.cpp
)
target_compile_definitions(SoftwareDownloadTest PRIVATE -DBOOTLOADER_BUILD)
target_compile_definitions(SoftwareDownloadTest PRIVATE -DBOOTLOADER_TEST)
