#include "software_download.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "hardware/flash.h"
#include "linker_definitions.h"
#include "pico_interface.h"
#include "types.h"

constexpr uint32_t MAX_REBOOT_DELAY{8388};

SoftwareDownload::SoftwareDownload(PicoInterface &pico_interface)
    : pages_flashed_{}, sectors_erased_{}, pico_interface_{pico_interface} {}

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

    pages_flashed_ = 0;
    sectors_erased_ = 0;

    return true;
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

    if ((pages_flashed_ * FLASH_PAGE_SIZE) > app_info.content.swap_app_size) {
        printf("Trying to flash more pages than expected, abort\n");
        return false;
    }

    auto status{true};
    if ((pages_flashed_ * FLASH_PAGE_SIZE) >=
        (sectors_erased_ * FLASH_SECTOR_SIZE)) {
        if (!pico_interface_.erase_flash(
                ADDR_WITH_XIP_OFFSET_AS_U32(SWAP_APP_ADDRESS) +
                    sectors_erased_ * FLASH_SECTOR_SIZE,
                FLASH_SECTOR_SIZE)) {
            status = false;
            printf(
                "Erasing app sectors failed at sector %zu and address %#x!\n",
                sectors_erased_,
                ADDR_WITH_XIP_OFFSET_AS_U32(SWAP_APP_ADDRESS) +
                    sectors_erased_ * FLASH_SECTOR_SIZE);
        } else {
            sectors_erased_++;
        }
    }
    if (status && pico_interface_.store_to_flash(
                      ADDR_WITH_XIP_OFFSET_AS_U32(SWAP_APP_ADDRESS) +
                          pages_flashed_ * FLASH_PAGE_SIZE,
                      binary_block, FLASH_PAGE_SIZE)) {
        pages_flashed_++;
    } else {
        printf("Write app chuck failed\n");
        status = false;
    }

    return status;
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
        printf(
            "Swap app hash verification successful, will reboot in "
            "1s...\n");
        uint32_t reboot_delay_ms{1000};
        reboot(reboot_delay_ms);
    } else {
        printf("Swap app hash verification failed\n");
        status = false;
    }

    return status;
}

auto SoftwareDownload::verify_app_hash() -> bool {
    app_info_t app_info{};
    read_app_info(app_info);
    return pico_interface_.verify_hash(app_info.content.app_hash,
                                       ADDR_AS_U32(APP_ADDRESS),
                                       app_info.content.app_size);
}
auto SoftwareDownload::verify_swap_app_hash() -> bool {
    app_info_t app_info{};
    read_app_info(app_info);
    return pico_interface_.verify_hash(app_info.content.swap_app_hash,
                                       ADDR_AS_U32(SWAP_APP_ADDRESS),
                                       app_info.content.swap_app_size);
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

    reboot(delay);

    return true;
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
