#ifndef PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_
#define PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_

#include <cstdint>

#include "hardware/flash.h"
#include "pico_interface.h"
#include "types.h"

constexpr unsigned int DOWNLOAD_BLOCK_SIZE{FLASH_PAGE_SIZE};

class SoftwareDownload {
   public:
    SoftwareDownload(PicoInterface &pico_interface);
    ~SoftwareDownload() = default;

    auto init_download(const uint32_t &size) -> bool;
    auto set_hash(const unsigned char app_hash[SHA256_DIGEST_SIZE]) const
        -> bool;
    auto write_app(const unsigned char binary_block[FLASH_PAGE_SIZE]) -> bool;
    auto download_complete() const -> bool;
    auto verify_app_hash() const -> bool;
    auto verify_swap_app_hash() const -> bool;
    void reboot(uint32_t delay_ms) const;
    auto restore(uint32_t delay_ms) const -> bool;

   private:
    static void read_app_info(app_info_t &app_info);
    auto write_app_info(app_info_t &app_info) const -> bool;
    uint32_t pages_flashed_;
    uint32_t sectors_erased_;
    PicoInterface &pico_interface_;
};

#endif  // PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_
