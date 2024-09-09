#include "bootloader.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "RP2040.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "linker_definitions.h"
#include "pico/stdlib.h"

Bootloader::Bootloader() { read_app_info(); }

void Bootloader::read_app_info() {
    memcpy(m_app_info.raw,
           reinterpret_cast<void *>(ADDR_AS_U32(__APP_INFO_ADDRESS)),
           FLASH_PAGE_SIZE);
}

void Bootloader::write_app_info() {
    uint32_t saved_interrupts = save_and_disable_interrupts();
    flash_range_erase(ADDR_WITH_XIP_OFFSET_AS_U32(__APP_INFO_ADDRESS),
                      FLASH_SECTOR_SIZE);
    flash_range_program(ADDR_WITH_XIP_OFFSET_AS_U32(__APP_INFO_ADDRESS),
                        m_app_info.raw, FLASH_PAGE_SIZE);
    restore_interrupts(saved_interrupts);
}

void Bootloader::start_user_app() {
    typedef void (*funcPtr)();

    auto vtor{ADDR_AS_U32(__APP_ADDRESS)};
    printf("Start app at %#X...\n", vtor);

    uint32_t reset_vector = *(volatile uint32_t *)(vtor + 0x04);
    auto app_main = (funcPtr)reset_vector;

    SCB->VTOR = (volatile uint32_t)(vtor);
    app_main();
}

void Bootloader::swap_app_images() {
    uint8_t swap_buffer_app[FLASH_SECTOR_SIZE];
    uint8_t swap_buffer_downloaded_app[FLASH_SECTOR_SIZE];

    const auto SECTORS_TO_SWAP{ADDR_AS_U32(__APP_LENGTH) / FLASH_SECTOR_SIZE};

    printf("Swap %u sectors\n", SECTORS_TO_SWAP);

    uint32_t saved_interrupts = save_and_disable_interrupts();
    for (uint16_t i{0}; i < SECTORS_TO_SWAP; i++) {
        memcpy(swap_buffer_app,
               reinterpret_cast<void *>(ADDR_AS_U32(__APP_ADDRESS) +
                                        i * FLASH_SECTOR_SIZE),
               FLASH_SECTOR_SIZE);
        memcpy(swap_buffer_downloaded_app,
               reinterpret_cast<void *>(ADDR_AS_U32(__SWAP_APP_ADDRESS) +
                                        i * FLASH_SECTOR_SIZE),
               FLASH_SECTOR_SIZE);
        flash_range_erase(
            ADDR_WITH_XIP_OFFSET_AS_U32(__APP_ADDRESS) + i * FLASH_SECTOR_SIZE,
            FLASH_SECTOR_SIZE);
        flash_range_erase(ADDR_WITH_XIP_OFFSET_AS_U32(__SWAP_APP_ADDRESS) +
                              i * FLASH_SECTOR_SIZE,
                          FLASH_SECTOR_SIZE);
        flash_range_program(
            ADDR_WITH_XIP_OFFSET_AS_U32(__APP_ADDRESS) + i * FLASH_SECTOR_SIZE,
            swap_buffer_downloaded_app, FLASH_SECTOR_SIZE);
        flash_range_program(ADDR_WITH_XIP_OFFSET_AS_U32(__SWAP_APP_ADDRESS) +
                                i * FLASH_SECTOR_SIZE,
                            swap_buffer_app, FLASH_SECTOR_SIZE);
    }
    restore_interrupts(saved_interrupts);

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
