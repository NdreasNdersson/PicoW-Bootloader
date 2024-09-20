cmake_minimum_required(VERSION 3.13...3.27)

enable_testing()

package_add_test(BootloaderTest bootloader_test.cpp ../src/bootloader.cpp)

include_directories(mocks)
include_directories(stubs)
include_directories(${CMAKE_SOURCE_DIR}/include_common)
include_directories(${CMAKE_SOURCE_DIR}/bootloader/src)
include_directories(${CMAKE_SOURCE_DIR}/bootloader_lib/src)
