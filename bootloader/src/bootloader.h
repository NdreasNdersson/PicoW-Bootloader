#ifndef PICO_BOOTLOADER_BOOTLOADER_H
#define PICO_BOOTLOADER_BOOTLOADER_H

#include <cstdint>

#include "bootloader_lib.h"
#include "linker_definitions.h"

class Bootloader {
   public:
    Bootloader();

    static void start_user_app();
    auto check_download_app_flag() const -> bool;
    auto check_restore_at_boot() const -> bool;
    void swap_app_images();

   private:
    void read_app_info();
    void write_app_info();

    app_info_t m_app_info;
};

#endif
