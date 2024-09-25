#include "bootloader.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "hardware/flash.h"
#include "linker_definitions.h"
#include "pico_interface.h"
#include "software_download.h"

constexpr uint32_t MAX_REBOOT_DELAY{8388};

Bootloader::Bootloader(PicoInterface &pico_interface)
    : m_pages_flashed{},
      pico_interface_{pico_interface},
      software_download_{pico_interface_} {}

auto Bootloader::init_download(const uint32_t &size) -> bool {
    return software_download_.init_download(size);
}

auto Bootloader::set_hash(const unsigned char app_hash[SHA256_DIGEST_SIZE])
    -> bool {
    return software_download_.set_hash(app_hash);
}

auto Bootloader::write_app(const unsigned char binary_block[FLASH_PAGE_SIZE])
    -> bool {
    return software_download_.write_app(binary_block);
}

auto Bootloader::download_complete() -> bool {
    return software_download_.download_complete();
}

auto Bootloader::verify_app_hash() -> bool {
    return software_download_.verify_app_hash();
}
auto Bootloader::verify_swap_app_hash() -> bool {
    return software_download_.verify_swap_app_hash();
}

void Bootloader::reboot(uint32_t delay) { software_download_.reboot(delay); }

auto Bootloader::restore(uint32_t delay) -> bool {
    return software_download_.restore(delay);
}

auto Bootloader::check_download_app_flag() const -> bool {
    app_info_t app_info{};
    read_app_info(app_info);
    return TRUE_MAGIC_NUMBER == app_info.content.app_downloaded;
}

auto Bootloader::check_restore_at_boot() const -> bool {
    app_info_t app_info{};
    read_app_info(app_info);
    if ((app_info.content.app_backed_up == TRUE_MAGIC_NUMBER) &&
        (app_info.content.app_restore_at_boot == TRUE_MAGIC_NUMBER)) {
        return true;
    } else {
        return false;
    }
}

void Bootloader::swap_app_images() {
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

auto Bootloader::write_app_info(app_info_t &app_info) -> bool {
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
