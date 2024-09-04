#ifndef PICOW_BOOTLOADER_BOOTLOADER_LIB_H
#define PICOW_BOOTLOADER_BOOTLOADER_LIB_H

#include <cstdint>

#include "hardware/flash.h"

constexpr uint8_t SHA256_DIGEST_SIZE{32U};
constexpr uint32_t TRUE_MAGIC_NUMBER{14253U};
constexpr uint32_t FALSE_NUMBER{0U};

union app_info_t {
    struct content_t {
        unsigned char app_hash[SHA256_DIGEST_SIZE];
        unsigned char swap_app_hash[SHA256_DIGEST_SIZE];
        uint32_t app_size;
        uint32_t swap_app_size;
        uint32_t app_downloaded;
        uint32_t app_backed_up;
    };
    struct content_t content;
    uint8_t raw[FLASH_PAGE_SIZE];
};

class SoftwareDownload {
   public:
    SoftwareDownload();

    void set_hash(const unsigned char app_hash[SHA256_DIGEST_SIZE]);
    void set_size(const uint32_t &size);
    auto write_app(const unsigned char binary_block[FLASH_PAGE_SIZE]) -> bool;
    void download_complete();

   private:
    void read_app_info();

    app_info_t m_app_info;
};

#endif
