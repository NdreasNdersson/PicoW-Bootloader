#ifndef PICO_BOOTLOADER_TYPES_H_
#define PICO_BOOTLOADER_TYPES_H_

#include "hardware/flash.h"

namespace PicoBootloader {

constexpr uint32_t TRUE_MAGIC_NUMBER{14253U};
constexpr uint32_t FALSE_NUMBER{0U};
constexpr uint8_t SHA256_DIGEST_SIZE{32U};

union app_info_t {
    struct content_t {
        unsigned char app_hash[SHA256_DIGEST_SIZE];
        unsigned char swap_app_hash[SHA256_DIGEST_SIZE];
        uint32_t app_size;
        uint32_t swap_app_size;
        uint32_t app_downloaded;
        uint32_t app_backed_up;
        uint32_t app_restore_at_boot;
    };
    struct content_t content;
    uint8_t raw[FLASH_PAGE_SIZE];
};
}  // namespace PicoBootloader

#endif  // PICO_BOOTLOADER_TYPES_H_
