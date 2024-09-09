#ifndef PICOW_BOOTLOADER_BOOTLOADER_H
#define PICOW_BOOTLOADER_BOOTLOADER_H

#include <cstdint>

#include "bootloader_lib.h"
#include "linker_definitions.h"

class Bootloader {
   public:
    Bootloader();

    static void start_user_app();
    static auto check_download_app_flag() -> bool {
        return TRUE_MAGIC_NUMBER ==
               (*((std::uint32_t *)ADDR_AS_U32(__APP_DOWNLOADED_FLAG_ADDRESS)));
    }
    void swap_app_images();

   private:
    void read_app_info();
    void write_app_info();

    app_info_t m_app_info;
};

#endif
