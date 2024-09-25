#ifndef PICO_BOOTLOADER_HAL_PICO_INTERFACE_IMPL_H_
#define PICO_BOOTLOADER_HAL_PICO_INTERFACE_IMPL_H_

#include <cstddef>
#include <cstdint>

#include "pico_interface.h"

using erase_flash_t = struct erase_flash_t_ {
    uint32_t flash_offs;
    size_t count;
};
using store_to_flash_t = struct stort_to_flash_t_ {
    uint32_t flash_offs;
    const uint8_t *data;
    size_t count;
};

class PicoInterfaceImpl : public PicoInterface {
   public:
    void watchdog_enable(uint32_t delay_ms, bool pause_on_debug) override;
    auto store_to_flash(uint32_t flash_offs, const uint8_t *data, size_t count)
        -> bool override;
    auto erase_flash(uint32_t flash_offs, size_t count) -> bool override;
    auto verify_hash(const unsigned char stored_sha256[SHA256_DIGEST_SIZE],
                     const uint32_t app_address,
                     const uint32_t app_size_address) -> bool override;

   private:
    static void program(void *data);
    static void erase(void *data);
};

#endif  // PICO_BOOTLOADER_HAL_PICO_INTERFACE_IMPL_H_
