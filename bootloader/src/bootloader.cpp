#include <hardware/sync.h>
#include <mbedtls/sha256.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>

#include "RP2040.h"
#include "common_definitions.h"
#include "hardware/flash.h"
#include "linker_definitions.h"
#include "pico/stdlib.h"

constexpr std::uint8_t SHA256_DIGEST_SIZE{32U};
constexpr std::uint32_t TRUE_MAGIC_NUMBER{14253U};
constexpr std::uint32_t FALSE_NUMBER{0U};

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

static void read_app_info(std::unique_ptr<app_info_t> &app_info) {
    memcpy(&app_info->raw[0], (void *)(ADDR_AS_U32(__APP_INFO_ADDRESS)),
           FLASH_PAGE_SIZE);
}

static void write_app_info(std::unique_ptr<app_info_t> &app_info) {
    uint32_t saved_interrupts = save_and_disable_interrupts();
    flash_range_erase(ADDR_WITH_XIP_OFFSET_AS_U32(__APP_INFO_ADDRESS),
                      FLASH_SECTOR_SIZE);
    flash_range_program(ADDR_WITH_XIP_OFFSET_AS_U32(__APP_INFO_ADDRESS),
                        app_info->raw, FLASH_PAGE_SIZE);
    restore_interrupts(saved_interrupts);
}

static auto verify_hash(const unsigned char stored_sha256[SHA256_DIGEST_SIZE],
                        const uint32_t app_address,
                        const uint32_t app_size_address) -> bool {
    /* printf("Verify app hash at %#X with calculated hash at %#X\n", hash_address, app_address); */

    /* unsigned char stored_sha256[SHA256_DIGEST_SIZE]; */
    /* memcpy(stored_sha256, (void *)hash_address, SHA256_DIGEST_SIZE); */

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

            puts("Calculated hash:");
            for (auto c : calculated_sha256) {
                printf("%x", c);
            }
            puts("");

            break;
        }
    }
    return hash_matched;
}

static void jump_to_vtor(const uint32_t vtor) {
    typedef void (*funcPtr)();

    printf("Start app at %#X...", vtor);

    uint32_t reset_vector = *(volatile uint32_t *)(vtor + 0x04);
    auto app_main = (funcPtr)reset_vector;

    SCB->VTOR = (volatile uint32_t)(vtor);
    app_main();
}

static auto check_download_app_flag() -> bool {
    return TRUE_MAGIC_NUMBER ==
           (*((std::uint32_t *)ADDR_AS_U32(__APP_DOWNLOADED_FLAG_ADDRESS)));
}

static void swap_app_images(std::unique_ptr<app_info_t> &app_info) {
    uint8_t swap_buffer_app[FLASH_SECTOR_SIZE];
    uint8_t swap_buffer_downloaded_app[FLASH_SECTOR_SIZE];

    const auto SECTORS_TO_SWAP{ADDR_AS_U32(__APP_LENGTH) / FLASH_SECTOR_SIZE};

    printf("Swap %u sectors\n", SECTORS_TO_SWAP);

    uint32_t saved_interrupts = save_and_disable_interrupts();
    for (uint16_t i{0}; i < SECTORS_TO_SWAP; i++) {
        memcpy(swap_buffer_app,
               (void *)(ADDR_AS_U32(__APP_ADDRESS) + i * FLASH_SECTOR_SIZE),
               FLASH_SECTOR_SIZE);
        memcpy(
            swap_buffer_downloaded_app,
            (void *)(ADDR_AS_U32(__SWAP_APP_ADDRESS) + i * FLASH_SECTOR_SIZE),
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
    memcpy(temp_hash, app_info->content.app_hash, SHA256_DIGEST_SIZE);
    memcpy(app_info->content.app_hash, app_info->content.swap_app_hash,
           SHA256_DIGEST_SIZE);
    memcpy(app_info->content.swap_app_hash, temp_hash, SHA256_DIGEST_SIZE);
    app_info->content.app_size = app_info->content.swap_app_size;
    app_info->content.swap_app_size = app_info->content.app_size;
    app_info->content.app_backed_up = TRUE_MAGIC_NUMBER;
    app_info->content.app_downloaded = FALSE_NUMBER;

    write_app_info(app_info);
}

static void print_welcome_message() {
    puts("");
    puts("******************************************************");
    puts("*                                                    *");
    puts("*           Raspberry Pi Pico W Bootloader           *");
    puts("*                                                    *");
    puts("******************************************************");
    puts("");
}

auto main() -> int {
    stdio_uart_init_full(PICO_UART, PICO_UART_BAUD_RATE, PICO_UART_TX_PIN,
                         PICO_UART_RX_PIN);
    print_welcome_message();
    sleep_ms(1000);

    assert(SHA256_DIGEST_SIZE == __APP_HASH_LENGTH);
    assert(4 == __APP_INFO_FLAG_LENGTH);
    assert(4 == __APP_SIZE_LENGTH);

    auto app_info = std::make_unique<app_info_t>();
    read_app_info(app_info);
    if (check_download_app_flag()) {
        puts("New app was downloaded!");
        if (verify_hash(app_info->content.swap_app_hash,
                        ADDR_AS_U32(__SWAP_APP_ADDRESS),
                        ADDR_AS_U32(__SWAP_APP_SIZE_ADDRESS))) {
            puts("New app hash was verified, swap images!");
            swap_app_images(app_info);
        } else {
            puts("New app hash verification FAILED!");
        }
    }

    if (verify_hash(app_info->content.app_hash, ADDR_AS_U32(__APP_ADDRESS),
                    ADDR_AS_U32(__APP_SIZE_ADDRESS))) {
        jump_to_vtor(ADDR_AS_U32(__APP_ADDRESS));
    }
    puts("Hash verification failed");
    while (true) {
    }

    return 0;
}
