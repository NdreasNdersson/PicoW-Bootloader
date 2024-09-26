#ifndef PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_
#define PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_

#include <cstdint>
#include <memory>

#ifdef BOOTLOADER_BUILD
#include "hal/pico_interface.h"
#endif
#include "hardware/flash.h"
#include "types.h"

constexpr unsigned int DOWNLOAD_BLOCK_SIZE{FLASH_PAGE_SIZE};

class SoftwareDownload {
   public:
    SoftwareDownload();

#ifdef BOOTLOADER_BUILD
    SoftwareDownload(PicoInterface *pico_interface);
#endif

    ~SoftwareDownload();

    auto init_download(const uint32_t &size) -> bool;
    auto set_hash(const unsigned char app_hash[SHA256_DIGEST_SIZE]) const
        -> bool;
    auto write_app(const unsigned char binary_block[FLASH_PAGE_SIZE]) -> bool;
    [[nodiscard]] auto download_complete() const -> bool;
    [[nodiscard]] auto verify_app_hash() const -> bool;
    [[nodiscard]] auto verify_swap_app_hash() const -> bool;
    void reboot(uint32_t delay_ms) const;
    [[nodiscard]] auto restore(uint32_t delay_ms) const -> bool;

   private:
    class SoftwareDownloadImpl;
    std::unique_ptr<SoftwareDownloadImpl> pimpl_;
};

#endif  // PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_
