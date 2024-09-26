#ifndef PICO_BOOTLOADER_BOOTLOADER_H_
#define PICO_BOOTLOADER_BOOTLOADER_H_

#include <cstdint>

#include "hal/pico_interface.h"
#include "software_download.h"
#include "types.h"

class Bootloader {
   public:
    Bootloader(PicoInterface &pico_interface);
    ~Bootloader() = default;

    auto verify_app_hash() const -> bool;
    auto verify_swap_app_hash() const -> bool;
    [[nodiscard]] static auto check_download_app_flag() -> bool;
    [[nodiscard]] static auto check_restore_at_boot() -> bool;
    void swap_app_images() const;

   private:
    static void read_app_info(app_info_t &app_info);
    auto write_app_info(app_info_t &app_info) const -> bool;

    uint32_t m_pages_flashed;
    PicoInterface &pico_interface_;
    SoftwareDownload software_download_;

    using flash_data_t = struct flash_data_t_ {
        const unsigned char *binary_block;
        uint32_t pages_flashed;
    };
};

#endif  // PICO_BOOTLOADER_BOOTLOADER_H_
