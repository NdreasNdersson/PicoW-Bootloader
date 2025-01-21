#ifndef PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_API_H_
#define PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_API_H_

#include <cstdint>

#include "hardware/flash.h"
#include "types.h"

namespace PicoBootloader {

class SoftwareDownloadApi {
   public:
    virtual auto init_download(const uint32_t &size) -> bool = 0;
    virtual auto set_hash(
        const unsigned char app_hash[SHA256_DIGEST_SIZE]) const -> bool = 0;
    virtual auto write_app(const unsigned char binary_block[FLASH_PAGE_SIZE])
        -> bool = 0;
    virtual auto download_complete() const -> bool = 0;
    virtual void reboot(uint32_t delay_ms) const = 0;
    virtual auto restore(uint32_t delay_ms) const -> bool = 0;
};
}  // namespace PicoBootloader

#endif  // PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_API_H_
