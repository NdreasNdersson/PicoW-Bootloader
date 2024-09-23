#ifndef PICO_BOOTLOADER_HARDWARE_FLASH_STUB_H
#define PICO_BOOTLOADER_HARDWARE_FLASH_STUB_H

#include <cstddef>
#include <cstdint>

constexpr unsigned int FLASH_PAGE_SIZE{256};
constexpr unsigned int FLASH_SECTOR_SIZE{1024};

void flash_range_program(uint32_t flash_offs, const uint8_t *data,
                         size_t count){};
void flash_range_erase(uint32_t flash_offs, size_t count){};

#endif  // PICO_BOOTLOADER_HARDWARE_FLASH_STUB_H
