#ifndef PICOW_BOOTLOADER_BOOTLOADER_H
#define PICOW_BOOTLOADER_BOOTLOADER_H

#include <cstdint>

#include "common_definitions.h"
#include "hardware/flash.h"
#include "linker_definitions.h"

constexpr std::uint8_t SHA256_DIGEST_SIZE{32U};
constexpr std::uint32_t TRUE_MAGIC_NUMBER{14253U};
constexpr std::uint32_t FALSE_NUMBER{0U};

class Bootloader {
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

   public:
    Bootloader();

    auto verify_app_hash() -> bool;
    auto verify_swap_app_hash() -> bool;
    static void jump_to_vtor(const uint32_t vtor);
    static auto check_download_app_flag() -> bool {
        return TRUE_MAGIC_NUMBER ==
               (*((std::uint32_t *)ADDR_AS_U32(__APP_DOWNLOADED_FLAG_ADDRESS)));
    }
    void swap_app_images();

   private:
    void read_app_info();
    const void write_app_info();
    static auto verify_hash(
        const unsigned char stored_sha256[SHA256_DIGEST_SIZE],
        const uint32_t app_address, const uint32_t app_size_address) -> bool;

    app_info_t m_app_info;
};

#endif
