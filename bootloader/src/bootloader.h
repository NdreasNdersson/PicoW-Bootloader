#ifndef PICO_BOOTLOADER_BOOTLOADER_H
#define PICO_BOOTLOADER_BOOTLOADER_H

#include "bootloader_lib.h"

class Bootloader : public SoftwareDownload {
   public:
    static void start_user_app();
    auto check_download_app_flag() const -> bool {
        return SoftwareDownload::check_download_app_flag();
    }
    auto check_restore_at_boot() const -> bool {
        return SoftwareDownload::check_restore_at_boot();
    }
    void swap_app_images() { SoftwareDownload::swap_app_images(); }
};

#endif
