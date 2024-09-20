#ifndef PICO_BOOTLOADER_BOOTLOADER_H
#define PICO_BOOTLOADER_BOOTLOADER_H

#include "bootloader_lib.h"

class Bootloader {
   public:
    Bootloader(BootloaderLib &bootloader_lib);

    auto check_download_app_flag() -> bool;
    auto check_restore_at_boot() -> bool;
    void swap_app_images();
    auto verify_app_hash() -> bool;
    auto verify_swap_app_hash() -> bool;

   private:
    BootloaderLib &bootloader_lib_;
};

#endif
