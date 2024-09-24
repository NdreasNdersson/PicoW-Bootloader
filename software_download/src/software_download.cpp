#include "software_download.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "hardware/flash.h"
#include "linker_definitions.h"

constexpr uint32_t MAX_REBOOT_DELAY{8388};

SoftwareDownload::SoftwareDownload(PicoInterface &pico_interface)
    : m_pages_flashed{}, pico_interface_{pico_interface} {}

auto SoftwareDownload::init_download(const uint32_t &size) -> bool {
    app_info_t app_info{};
    read_app_info(app_info);
    std::memset(app_info.content.swap_app_hash, 0, SHA256_DIGEST_SIZE);
    app_info.content.swap_app_size = size;
    app_info.content.app_backed_up = FALSE_NUMBER;
    app_info.content.app_downloaded = FALSE_NUMBER;
    if (!write_app_info(app_info)) {
        printf("Write app info failed\n");
        return false;
    }

    m_pages_flashed = 0;

    auto status{true};
    const auto sectors_to_erase{(size + FLASH_SECTOR_SIZE - 1) /
                                FLASH_SECTOR_SIZE};
    for (size_t i{0}; i < sectors_to_erase; i++) {
        if (!pico_interface_.erase_flash(
                ADDR_WITH_XIP_OFFSET_AS_U32(SWAP_APP_ADDRESS) +
                    i * FLASH_SECTOR_SIZE,
                FLASH_SECTOR_SIZE)) {
            status = false;
            printf("Erasing app sectors failed!\n");
            break;
        }
    }

    return status;
}

auto SoftwareDownload::set_hash(
    const unsigned char app_hash[SHA256_DIGEST_SIZE]) -> bool {
    app_info_t app_info{};
    read_app_info(app_info);
    std::memcpy(app_info.content.swap_app_hash, app_hash, SHA256_DIGEST_SIZE);

    auto status{true};
    if (!write_app_info(app_info)) {
        printf("Write app info failed\n");
        status = false;
    }

    return status;
}

auto SoftwareDownload::write_app(
    const unsigned char binary_block[FLASH_PAGE_SIZE]) -> bool {
    app_info_t app_info{};
    read_app_info(app_info);

    if ((m_pages_flashed * FLASH_PAGE_SIZE) > app_info.content.swap_app_size) {
        return false;
    }

    if (!pico_interface_.store_to_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(SWAP_APP_ADDRESS) +
                m_pages_flashed * FLASH_PAGE_SIZE,
            binary_block, FLASH_PAGE_SIZE)) {
        printf("Write app chuck failed\n");
        return false;
    } else {
        m_pages_flashed++;
    }

    return true;
}

auto SoftwareDownload::download_complete() -> bool {
    app_info_t app_info{};
    read_app_info(app_info);
    app_info.content.app_downloaded = TRUE_MAGIC_NUMBER;
    if (!write_app_info(app_info)) {
        printf("Write app info failed\n");
        return false;
    }

    auto status{true};
    if (verify_swap_app_hash()) {
        printf("Swap app hash verification successful, will reboot in 1s...\n");
        uint32_t reboot_delay_ms{1000};
        reboot(reboot_delay_ms);
    } else {
        printf("Swap app hash verification failed\n");
        status = false;
    }

    return false;
}

auto SoftwareDownload::verify_app_hash() -> bool {
    app_info_t app_info{};
    read_app_info(app_info);
    return pico_interface_.verify_hash(app_info.content.app_hash,
                                       ADDR_AS_U32(APP_ADDRESS),
                                       ADDR_AS_U32(APP_SIZE_ADDRESS));
}
auto SoftwareDownload::verify_swap_app_hash() -> bool {
    app_info_t app_info{};
    read_app_info(app_info);
    return pico_interface_.verify_hash(app_info.content.swap_app_hash,
                                       ADDR_AS_U32(SWAP_APP_ADDRESS),
                                       ADDR_AS_U32(SWAP_APP_SIZE_ADDRESS));
}

void SoftwareDownload::reboot(uint32_t delay) {
    if (delay > MAX_REBOOT_DELAY) {
        delay = MAX_REBOOT_DELAY;
    }
    pico_interface_.watchdog_enable(delay, true);
}

auto SoftwareDownload::restore(uint32_t delay) -> bool {
    app_info_t app_info{};
    read_app_info(app_info);
    if (app_info.content.app_backed_up != TRUE_MAGIC_NUMBER) {
        return false;
    }

    app_info.content.app_restore_at_boot = TRUE_MAGIC_NUMBER;
    if (!write_app_info(app_info)) {
        printf("Write app info failed\n");
        return false;
    }

    if (delay > MAX_REBOOT_DELAY) {
        delay = MAX_REBOOT_DELAY;
    }
    pico_interface_.watchdog_enable(delay, true);

    return true;
}

auto SoftwareDownload::check_download_app_flag() const -> bool {
    app_info_t app_info{};
    read_app_info(app_info);
    return TRUE_MAGIC_NUMBER == app_info.content.app_downloaded;
}

auto SoftwareDownload::check_restore_at_boot() const -> bool {
    app_info_t app_info{};
    read_app_info(app_info);
    if ((app_info.content.app_backed_up == TRUE_MAGIC_NUMBER) &&
        (app_info.content.app_restore_at_boot == TRUE_MAGIC_NUMBER)) {
        return true;
    } else {
        return false;
    }
}

void SoftwareDownload::swap_app_images() {
    uint8_t swap_buffer_app[FLASH_SECTOR_SIZE];
    uint8_t swap_buffer_downloaded_app[FLASH_SECTOR_SIZE];

    const auto SECTORS_TO_SWAP{ADDR_AS_U32(APP_LENGTH) / FLASH_SECTOR_SIZE};

    for (size_t i{0}; i < SECTORS_TO_SWAP; i++) {
        memcpy(swap_buffer_app,
               reinterpret_cast<void *>(ADDR_AS_U32(APP_ADDRESS) +
                                        i * FLASH_SECTOR_SIZE),
               FLASH_SECTOR_SIZE);
        memcpy(swap_buffer_downloaded_app,
               reinterpret_cast<void *>(ADDR_AS_U32(SWAP_APP_ADDRESS) +
                                        i * FLASH_SECTOR_SIZE),
               FLASH_SECTOR_SIZE);
        pico_interface_.erase_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(APP_ADDRESS) + i * FLASH_SECTOR_SIZE,
            FLASH_SECTOR_SIZE);
        pico_interface_.erase_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(SWAP_APP_ADDRESS) +
                i * FLASH_SECTOR_SIZE,
            FLASH_SECTOR_SIZE);
        pico_interface_.store_to_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(APP_ADDRESS) + i * FLASH_SECTOR_SIZE,
            swap_buffer_downloaded_app, FLASH_SECTOR_SIZE);
        pico_interface_.store_to_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(SWAP_APP_ADDRESS) +
                i * FLASH_SECTOR_SIZE,
            swap_buffer_app, FLASH_SECTOR_SIZE);
    }

    app_info_t app_info{};
    read_app_info(app_info);
    // Update app info
    {
        unsigned char temp_hash[SHA256_DIGEST_SIZE];
        memcpy(temp_hash, app_info.content.app_hash, SHA256_DIGEST_SIZE);
        memcpy(app_info.content.app_hash, app_info.content.swap_app_hash,
               SHA256_DIGEST_SIZE);
        memcpy(app_info.content.swap_app_hash, temp_hash, SHA256_DIGEST_SIZE);
    }

    {
        auto temp_size{app_info.content.app_size};
        app_info.content.app_size = app_info.content.swap_app_size;
        app_info.content.swap_app_size = temp_size;
    }

    app_info.content.app_backed_up = TRUE_MAGIC_NUMBER;
    app_info.content.app_downloaded = FALSE_NUMBER;
    app_info.content.app_restore_at_boot = FALSE_NUMBER;

    if (!write_app_info(app_info)) {
        printf("Write app info failed\n");
    }
}

void SoftwareDownload::read_app_info(app_info_t &app_info) {
    std::memcpy(app_info.raw, g_app_info, FLASH_PAGE_SIZE);
}

auto SoftwareDownload::write_app_info(app_info_t &app_info) -> bool {
    if (!pico_interface_.erase_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(APP_INFO_ADDRESS), FLASH_SECTOR_SIZE)) {
        printf("Bootloader lib flash safe execute failed\n");
        return false;
    }
    if (!pico_interface_.store_to_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(APP_INFO_ADDRESS),
            static_cast<uint8_t *>(app_info.raw), FLASH_PAGE_SIZE)) {
        printf("Bootloader lib flash safe execute failed\n");
        return false;
    }

    return true;
}
