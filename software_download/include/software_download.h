#ifndef PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_
#define PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_

#include <cstdint>
#include <memory>

#include "software_download_api.h"

#ifdef BOOTLOADER_BUILD
#include "hal/pico_interface.h"
#endif
#include "hardware/flash.h"
#include "types.h"

namespace PicoBootloader {

class SoftwareDownload : public SoftwareDownloadApi {
   public:
    SoftwareDownload();

#ifdef BOOTLOADER_BUILD
    SoftwareDownload(PicoInterface *pico_interface);
#endif

    ~SoftwareDownload();

    auto init_download(const uint32_t &size) -> bool override;
    auto set_hash(const unsigned char app_hash[SHA256_DIGEST_SIZE]) const
        -> bool override;
    auto write_app(const unsigned char binary_block[FLASH_PAGE_SIZE])
        -> bool override;
    [[nodiscard]] auto download_complete() const -> bool override;
    [[nodiscard]] auto verify_app_hash() const -> bool;
    [[nodiscard]] auto verify_swap_app_hash() const -> bool;
    void reboot(uint32_t delay_ms) const override;
    [[nodiscard]] auto restore(uint32_t delay_ms) const -> bool override;

   private:
    class SoftwareDownloadImpl;
    std::unique_ptr<SoftwareDownloadImpl> pimpl_;
};
}  // namespace PicoBootloader

#endif  // PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_
