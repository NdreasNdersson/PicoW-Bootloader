#include "bootloader.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "hardware/flash.h"
#include "linker_definitions.h"

Bootloader::Bootloader(PicoInterface *pico_interface)
    : m_pages_flashed{},
      pico_interface_{pico_interface},
      software_download_{pico_interface_} {}

auto Bootloader::verify_app_hash() const -> bool {
    return software_download_.verify_app_hash();
}
auto Bootloader::verify_swap_app_hash() const -> bool {
    return software_download_.verify_swap_app_hash();
}
auto Bootloader::check_download_app_flag() -> bool {
    app_info_t app_info{};
    read_app_info(app_info);
    return TRUE_MAGIC_NUMBER == app_info.content.app_downloaded;
}

auto Bootloader::check_restore_at_boot() -> bool {
    app_info_t app_info{};
    read_app_info(app_info);
    if ((app_info.content.app_backed_up == TRUE_MAGIC_NUMBER) &&
        (app_info.content.app_restore_at_boot == TRUE_MAGIC_NUMBER)) {
        return true;
    } else {
        return false;
    }
}

void Bootloader::swap_app_images() const {
    uint8_t swap_buffer_app[FLASH_SECTOR_SIZE]{};
    uint8_t swap_buffer_downloaded_app[FLASH_SECTOR_SIZE]{};

    app_info_t app_info{};
    read_app_info(app_info);
    auto size{
        std::max(app_info.content.app_size, app_info.content.swap_app_size)};

    const auto SECTORS_TO_SWAP{(size + FLASH_SECTOR_SIZE - 1) /
                               FLASH_SECTOR_SIZE};

    for (size_t i{0}; i < SECTORS_TO_SWAP; i++) {
        memcpy(swap_buffer_app,
               reinterpret_cast<void *>(ADDR_AS_U32(APP_ADDRESS) +
                                        i * FLASH_SECTOR_SIZE),
               FLASH_SECTOR_SIZE);
        memcpy(swap_buffer_downloaded_app,
               reinterpret_cast<void *>(ADDR_AS_U32(SWAP_APP_ADDRESS) +
                                        i * FLASH_SECTOR_SIZE),
               FLASH_SECTOR_SIZE);
        pico_interface_->erase_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(APP_ADDRESS) + i * FLASH_SECTOR_SIZE,
            FLASH_SECTOR_SIZE);
        pico_interface_->erase_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(SWAP_APP_ADDRESS) +
                i * FLASH_SECTOR_SIZE,
            FLASH_SECTOR_SIZE);
        pico_interface_->store_to_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(APP_ADDRESS) + i * FLASH_SECTOR_SIZE,
            swap_buffer_downloaded_app, FLASH_SECTOR_SIZE);
        pico_interface_->store_to_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(SWAP_APP_ADDRESS) +
                i * FLASH_SECTOR_SIZE,
            swap_buffer_app, FLASH_SECTOR_SIZE);
    }

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

void Bootloader::read_app_info(app_info_t &app_info) {
    std::memcpy(app_info.raw, g_app_info, FLASH_PAGE_SIZE);
}

auto Bootloader::write_app_info(app_info_t &app_info) const -> bool {
    if (!pico_interface_->erase_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(APP_INFO_ADDRESS), FLASH_SECTOR_SIZE)) {
        printf("Bootloader lib flash safe execute failed\n");
        return false;
    }
    if (!pico_interface_->store_to_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(APP_INFO_ADDRESS),
            static_cast<uint8_t *>(app_info.raw), FLASH_PAGE_SIZE)) {
        printf("Bootloader lib flash safe execute failed\n");
        return false;
    }

    return true;
}
