#ifndef PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_
#define PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_

#include <cstdint>

#include "bootloader.h"
#include "hardware/flash.h"

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
        uint32_t app_restore_at_boot;
    };
    struct content_t content;
    uint8_t raw[FLASH_PAGE_SIZE];
};

class SoftwareDownload : public Bootloader {
   public:
    SoftwareDownload();
    virtual ~SoftwareDownload();

    void init_download(const uint32_t &size) override;
    void set_hash(const unsigned char app_hash[SHA256_DIGEST_SIZE]) override;
    auto write_app(const unsigned char binary_block[FLASH_PAGE_SIZE])
        -> bool override;
    void download_complete() override;
    auto verify_app_hash() -> bool override;
    auto verify_swap_app_hash() -> bool override;
    void reboot(uint32_t delay) override;
    auto restore(uint32_t delay) -> bool override;

   protected:
    auto check_download_app_flag() const -> bool override;
    auto check_restore_at_boot() const -> bool override;
    void swap_app_images() override;

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

#endif  // PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_
