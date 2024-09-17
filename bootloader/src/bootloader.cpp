#include "bootloader.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "RP2040.h"
#include "hardware/flash.h"
#include "linker_definitions.h"
#include "pico/stdlib.h"

Bootloader::Bootloader() { read_app_info(); }

void Bootloader::read_app_info() {
    memcpy(m_app_info.raw, g_app_info, FLASH_PAGE_SIZE);
}

void Bootloader::write_app_info() {
    flash_range_erase(ADDR_WITH_XIP_OFFSET_AS_U32(APP_INFO_ADDRESS),
                      FLASH_SECTOR_SIZE);
    flash_range_program(ADDR_WITH_XIP_OFFSET_AS_U32(APP_INFO_ADDRESS),
                        m_app_info.raw, FLASH_PAGE_SIZE);
}

void Bootloader::start_user_app() {
    typedef void (*funcPtr)();

    auto vtor{ADDR_AS_U32(APP_ADDRESS)};
    printf("Start app at %#X...\n", vtor);

    uint32_t reset_vector = *(volatile uint32_t *)(vtor + 0x04);
    auto app_main = (funcPtr)reset_vector;

    SCB->VTOR = (volatile uint32_t)(vtor);
    app_main();
}

void Bootloader::swap_app_images() {
    uint8_t swap_buffer_app[FLASH_SECTOR_SIZE];
    uint8_t swap_buffer_downloaded_app[FLASH_SECTOR_SIZE];

    const auto SECTORS_TO_SWAP{ADDR_AS_U32(APP_LENGTH) / FLASH_SECTOR_SIZE};

    printf("Swap %u sectors\n", SECTORS_TO_SWAP);

    for (size_t i{0}; i < SECTORS_TO_SWAP; i++) {
        memcpy(swap_buffer_app,
               reinterpret_cast<void *>(ADDR_AS_U32(APP_ADDRESS) +
                                        i * FLASH_SECTOR_SIZE),
               FLASH_SECTOR_SIZE);
        memcpy(swap_buffer_downloaded_app,
               reinterpret_cast<void *>(ADDR_AS_U32(SWAP_APP_ADDRESS) +
                                        i * FLASH_SECTOR_SIZE),
               FLASH_SECTOR_SIZE);
        flash_range_erase(
            ADDR_WITH_XIP_OFFSET_AS_U32(APP_ADDRESS) + i * FLASH_SECTOR_SIZE,
            FLASH_SECTOR_SIZE);
        flash_range_erase(ADDR_WITH_XIP_OFFSET_AS_U32(SWAP_APP_ADDRESS) +
                              i * FLASH_SECTOR_SIZE,
                          FLASH_SECTOR_SIZE);
        flash_range_program(
            ADDR_WITH_XIP_OFFSET_AS_U32(APP_ADDRESS) + i * FLASH_SECTOR_SIZE,
            swap_buffer_downloaded_app, FLASH_SECTOR_SIZE);
        flash_range_program(ADDR_WITH_XIP_OFFSET_AS_U32(SWAP_APP_ADDRESS) +
                                i * FLASH_SECTOR_SIZE,
                            swap_buffer_app, FLASH_SECTOR_SIZE);
    }

    // Update app info
    unsigned char temp_hash[SHA256_DIGEST_SIZE];
    memcpy(temp_hash, m_app_info.content.app_hash, SHA256_DIGEST_SIZE);
    memcpy(m_app_info.content.app_hash, m_app_info.content.swap_app_hash,
           SHA256_DIGEST_SIZE);
    memcpy(m_app_info.content.swap_app_hash, temp_hash, SHA256_DIGEST_SIZE);
    m_app_info.content.app_size = m_app_info.content.swap_app_size;
    m_app_info.content.swap_app_size = m_app_info.content.app_size;
    m_app_info.content.app_backed_up = TRUE_MAGIC_NUMBER;
    m_app_info.content.app_downloaded = FALSE_NUMBER;

    write_app_info();
}
