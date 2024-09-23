cmake_minimum_required(VERSION 3.13...3.27)

enable_testing()

include_directories(mocks)
include_directories(stubs)
include_directories(${CMAKE_SOURCE_DIR}/include_common)
include_directories(${CMAKE_SOURCE_DIR}/software_download/src)
include_directories(${CMAKE_SOURCE_DIR}/software_download/include)

package_add_test(BootloaderTest
    bootloader_test.cpp
    ${CMAKE_SOURCE_DIR}/software_download/src/software_download.cpp
)
