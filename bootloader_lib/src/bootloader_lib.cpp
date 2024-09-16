#include "bootloader_lib.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/watchdog.h"
#include "linker_definitions.h"
#include "mbedtls/sha256.h"
#include "pico/flash.h"

constexpr uint32_t MAX_REBOOT_DELAY{8388};

SoftwareDownload::SoftwareDownload() : m_app_info{}, m_pages_flashed{} {
    read_app_info();
}

void SoftwareDownload::init_download(const uint32_t &size) {
    std::memset(m_app_info.content.swap_app_hash, 0, SHA256_DIGEST_SIZE);
    m_app_info.content.swap_app_size = size;
    m_app_info.content.app_backed_up = FALSE_NUMBER;
    m_app_info.content.app_downloaded = FALSE_NUMBER;
    write_app_info();

    m_pages_flashed = 0;
    auto status{flash_safe_execute(&erase_swap, nullptr, 100U)};
    if (PICO_OK != status) {
        printf("Bootloader lib flash safe execute failed with code: %d!",
               status);
    }
}

void SoftwareDownload::set_hash(
    const unsigned char app_hash[SHA256_DIGEST_SIZE]) {
    std::memcpy(m_app_info.content.swap_app_hash, app_hash, SHA256_DIGEST_SIZE);
}

auto SoftwareDownload::write_app(
    const unsigned char binary_block[FLASH_PAGE_SIZE]) -> bool {
    if ((m_pages_flashed * FLASH_PAGE_SIZE) > ADDR_AS_U32(APP_LENGTH)) {
        return false;
    }

    flash_data_t data{};
    data.binary_block = binary_block;
    data.pages_flashed = m_pages_flashed;
    auto status{flash_safe_execute(&program, static_cast<void *>(&data), 100U)};
    if (PICO_OK != status) {
        printf("Bootloader lib flash safe execute failed with code: %d!",
               status);
        return false;
    }
    m_pages_flashed++;
    return true;
}

void SoftwareDownload::download_complete() {
    m_app_info.content.app_downloaded = TRUE_MAGIC_NUMBER;
    write_app_info();

    if (verify_swap_app_hash()) {
        printf("Swap app hash verification successful, will reboot in 1s...");
        uint32_t reboot_delay_ms{1000};
        reboot(reboot_delay_ms);
    } else {
        printf("Swap app hash verification failed");
    }
}

auto SoftwareDownload::verify_app_hash() -> bool {
    read_app_info();
    return verify_hash(m_app_info.content.app_hash, ADDR_AS_U32(APP_ADDRESS),
                       ADDR_AS_U32(APP_SIZE_ADDRESS));
}
auto SoftwareDownload::verify_swap_app_hash() -> bool {
    read_app_info();
    return verify_hash(m_app_info.content.swap_app_hash,
                       ADDR_AS_U32(SWAP_APP_ADDRESS),
                       ADDR_AS_U32(SWAP_APP_SIZE_ADDRESS));
}

void SoftwareDownload::reboot(uint32_t delay) {
    if (delay > MAX_REBOOT_DELAY) {
        delay = MAX_REBOOT_DELAY;
    }
    watchdog_enable(delay, true);
}

auto SoftwareDownload::verify_hash(
    const unsigned char stored_sha256[SHA256_DIGEST_SIZE],
    const uint32_t app_address, const uint32_t app_size_address) -> bool {
    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);

    int ret;
    ret = mbedtls_sha256_starts_ret(&sha256_ctx, 0);
    if (ret) {
        return false;
    }

    auto image_size = (size_t) * ((std::uint32_t *)app_size_address);
    ret = mbedtls_sha256_update_ret(
        &sha256_ctx, (const unsigned char *)app_address, image_size);
    if (ret) {
        return false;
    }

    unsigned char calculated_sha256[SHA256_DIGEST_SIZE];
    ret = mbedtls_sha256_finish_ret(&sha256_ctx, calculated_sha256);
    if (ret) {
        return false;
    }

    mbedtls_sha256_free(&sha256_ctx);

    // Compare calculated and stored hash
    auto hash_matched{true};
    for (int i = 0; i < SHA256_DIGEST_SIZE; i++) {
        if (stored_sha256[i] != calculated_sha256[i]) {
            hash_matched = false;
            break;
        }
    }
    return hash_matched;
}
void SoftwareDownload::read_app_info() {
    std::memcpy(m_app_info.raw, g_app_info, FLASH_PAGE_SIZE);
}

void SoftwareDownload::write_app_info() {
    auto status{flash_safe_execute(&erase_and_program_app_info,
                                   static_cast<void *>(&m_app_info.raw), 100U)};
    if (PICO_OK != status) {
        printf("Bootloader lib flash safe execute failed with code: %d!",
               status);
    }
}

void SoftwareDownload::erase_and_program_app_info(void *data) {
    flash_range_erase(ADDR_WITH_XIP_OFFSET_AS_U32(APP_INFO_ADDRESS),
                      FLASH_SECTOR_SIZE);
    flash_range_program(ADDR_WITH_XIP_OFFSET_AS_U32(APP_INFO_ADDRESS),
                        static_cast<uint8_t *>(data), FLASH_PAGE_SIZE);
}

void SoftwareDownload::program(void *data) {
    auto flash_data{static_cast<flash_data_t *>(data)};
    flash_range_program(ADDR_WITH_XIP_OFFSET_AS_U32(SWAP_APP_ADDRESS) +
                            flash_data->pages_flashed * FLASH_PAGE_SIZE,
                        flash_data->binary_block, FLASH_PAGE_SIZE);
}

void SoftwareDownload::erase_swap(void * /*data*/) {
    const auto sectors_to_erase{ADDR_AS_U32(APP_LENGTH) / FLASH_SECTOR_SIZE};
    for (size_t i{0}; i < sectors_to_erase; i++) {
        flash_range_erase(ADDR_WITH_XIP_OFFSET_AS_U32(SWAP_APP_ADDRESS) +
                              i * FLASH_SECTOR_SIZE,
                          FLASH_SECTOR_SIZE);
    }
}
