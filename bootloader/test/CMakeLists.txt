cmake_minimum_required(VERSION 3.13...3.27)

enable_testing()

package_add_test(BootloaderTest bootloader_test.cpp)

include_directories(stubs)
include_directories(../src)
