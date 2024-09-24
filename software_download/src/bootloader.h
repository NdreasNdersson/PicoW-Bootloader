#ifndef PICO_BOOTLOADER_BOOTLOADER_H_
#define PICO_BOOTLOADER_BOOTLOADER_H_

#include <cstdint>

#include "hardware/flash.h"
#include "pico_interface.h"

constexpr unsigned int DOWNLOAD_BLOCK_SIZE{FLASH_PAGE_SIZE};

class Bootloader {
   public:
    virtual ~Bootloader() = default;
    virtual auto init_download(const uint32_t &size) -> bool = 0;
    virtual auto set_hash(const unsigned char app_hash[SHA256_DIGEST_SIZE])
        -> bool = 0;
    virtual auto write_app(const unsigned char binary_block[FLASH_PAGE_SIZE])
        -> bool = 0;
    virtual auto download_complete() -> bool = 0;
    virtual auto verify_app_hash() -> bool = 0;
    virtual auto verify_swap_app_hash() -> bool = 0;
    virtual void reboot(uint32_t delay) = 0;
    virtual auto restore(uint32_t delay) -> bool = 0;
    virtual auto check_download_app_flag() const -> bool = 0;
    virtual auto check_restore_at_boot() const -> bool = 0;
    virtual void swap_app_images() = 0;
};

#endif  // PICO_BOOTLOADER_BOOTLOADER_H_
