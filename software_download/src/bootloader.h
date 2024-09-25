#ifndef PICO_BOOTLOADER_BOOTLOADER_H_
#define PICO_BOOTLOADER_BOOTLOADER_H_

#include <cstdint>

#include "hardware/flash.h"
#include "pico_interface.h"
#include "software_download.h"
#include "types.h"

class Bootloader {
   public:
    Bootloader(PicoInterface &pico_interface);
    ~Bootloader() = default;

    auto init_download(const uint32_t &size) -> bool;
    auto set_hash(const unsigned char app_hash[SHA256_DIGEST_SIZE]) -> bool;
    auto write_app(const unsigned char binary_block[FLASH_PAGE_SIZE]) -> bool;
    auto download_complete() -> bool;
    auto verify_app_hash() -> bool;
    auto verify_swap_app_hash() -> bool;
    void reboot(uint32_t delay);
    auto restore(uint32_t delay) -> bool;
    [[nodiscard]] auto check_download_app_flag() const -> bool;
    [[nodiscard]] auto check_restore_at_boot() const -> bool;
    void swap_app_images();

   private:
    static void read_app_info(app_info_t &app_info);
    auto write_app_info(app_info_t &app_info) -> bool;

    uint32_t m_pages_flashed;
    PicoInterface &pico_interface_;
    SoftwareDownload software_download_;

    using flash_data_t = struct flash_data_t_ {
        const unsigned char *binary_block;
        uint32_t pages_flashed;
    };
};

#endif  // PICO_BOOTLOADER_BOOTLOADER_H_
