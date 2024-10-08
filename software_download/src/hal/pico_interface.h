#ifndef PICO_BOOTLOADER_PICO_INTERFACE_H_
#define PICO_BOOTLOADER_PICO_INTERFACE_H_

#include <cstddef>
#include <cstdint>

#include "types.h"

namespace PicoBootloader {

class PicoInterface {
   public:
    virtual void reboot(uint32_t delay_ms) = 0;
    virtual auto store_to_flash(uint32_t flash_offs, const uint8_t *data,
                                size_t count) -> bool = 0;
    virtual auto erase_flash(uint32_t flash_offs, size_t count) -> bool = 0;
    virtual auto verify_hash(
        const unsigned char stored_sha256[SHA256_DIGEST_SIZE],
        const uint32_t app_address, const uint32_t app_size) -> bool = 0;
};

}  // namespace PicoBootloader

#endif  // PICO_BOOTLOADER_PICO_INTERFACE_H_
