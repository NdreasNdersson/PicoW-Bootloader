#ifndef PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_
#define PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_

#include <cstdint>

#include "bootloader.h"
#include "pico_interface.h"

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
    SoftwareDownload(PicoInterface &pico_interface);
    ~SoftwareDownload() = default;

    auto init_download(const uint32_t &size) -> bool override;
    auto set_hash(const unsigned char app_hash[SHA256_DIGEST_SIZE])
        -> bool override;
    auto write_app(const unsigned char binary_block[FLASH_PAGE_SIZE])
        -> bool override;
    auto download_complete() -> bool override;
    auto verify_app_hash() -> bool override;
    auto verify_swap_app_hash() -> bool override;
    void reboot(uint32_t delay) override;
    auto restore(uint32_t delay) -> bool override;

   protected:
    auto check_download_app_flag() const -> bool override;
    auto check_restore_at_boot() const -> bool override;
    void swap_app_images() override;

   private:
    static void read_app_info(app_info_t &app_info);
    auto write_app_info(app_info_t &app_info) -> bool;

    uint32_t m_pages_flashed;
    PicoInterface &pico_interface_;

    using flash_data_t = struct flash_data_t_ {
        const unsigned char *binary_block;
        uint32_t pages_flashed;
    };
};

#endif  // PICO_BOOTLOADER_SOFTWARE_DOWNLOAD_H_
