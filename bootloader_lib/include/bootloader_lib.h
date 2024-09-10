#ifndef PICOW_BOOTLOADER_BOOTLOADER_LIB_H
#define PICOW_BOOTLOADER_BOOTLOADER_LIB_H

#include <cstdint>

#include "hardware/flash.h"

constexpr uint8_t SHA256_DIGEST_SIZE{32U};
constexpr uint32_t TRUE_MAGIC_NUMBER{14253U};
constexpr uint32_t FALSE_NUMBER{0U};
constexpr unsigned int DOWNLOAD_BLOCK_SIZE{FLASH_PAGE_SIZE};

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

    void init_download(const uint32_t &size);
    void set_hash(const unsigned char app_hash[SHA256_DIGEST_SIZE]);
    auto write_app(const unsigned char binary_block[FLASH_PAGE_SIZE]) -> bool;
    void download_complete();
    auto verify_app_hash() -> bool;
    auto verify_swap_app_hash() -> bool;
    void reboot(uint32_t delay);

   private:
    static auto verify_hash(
        const unsigned char stored_sha256[SHA256_DIGEST_SIZE],
        const uint32_t app_address, const uint32_t app_size_address) -> bool;
    void read_app_info();
    void write_app_info();
    static void erase_and_program_app_info(void *data);
    static void program(void *data);
    static void erase_swap(void *data);

    app_info_t m_app_info;
    uint32_t m_pages_flashed;

    using flash_data_t = struct flash_data_t_ {
        const unsigned char *binary_block;
        uint32_t pages_flashed;
    };
};

#endif
