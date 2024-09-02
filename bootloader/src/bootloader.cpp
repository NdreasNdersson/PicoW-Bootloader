#include <hardware/sync.h>
#include <mbedtls/sha256.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "RP2040.h"
#include "common_definitions.h"
#include "hardware/flash.h"
#include "linker_definitions.h"
#include "pico/stdlib.h"

constexpr std::uint8_t SHA256_DIGEST_SIZE{32U};
constexpr std::uint32_t TRUE_MAGIC_NUMBER{14253U};
constexpr std::uint32_t FALSE_NUMBER{0U};

static auto verify_hash(const uint32_t hash_address, const uint32_t app_address,
                        const uint32_t app_size_address) -> bool {
    printf("Verify app hash at %#X...\n", hash_address);

    unsigned char stored_sha256[SHA256_DIGEST_SIZE];
    memcpy(stored_sha256, (void *)hash_address, SHA256_DIGEST_SIZE);

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

            puts("Stored hash:");
            for (auto c : stored_sha256) {
                printf("%x", c);
            }
            puts("");
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
           (*((std::uint32_t *)ADDR_AS_U32(__DAPP_DOWNLOAD_FLAG_ADDRESS)));
}

static void swap_app_images() {
    uint8_t swap_buffer_app[FLASH_SECTOR_SIZE];
    uint8_t swap_buffer_downloaded_app[FLASH_SECTOR_SIZE];

    const auto SECTORS_TO_SWAP{
        (ADDR_AS_U32(__APP_LENGTH) + ADDR_AS_U32(__APP_INFO_LENGTH)) /
        FLASH_SECTOR_SIZE};

    printf("Swap %u sectors\n", SECTORS_TO_SWAP);

    uint32_t saved_interrupts = save_and_disable_interrupts();
    // First sector is app info, modify download and restore flags
    auto app_info{true};
    for (uint16_t i{0}; i < SECTORS_TO_SWAP; i++) {
        memcpy(
            swap_buffer_app,
            (void *)(ADDR_AS_U32(__APP_INFO_ADDRESS) + i * FLASH_SECTOR_SIZE),
            FLASH_SECTOR_SIZE);
        memcpy(
            swap_buffer_downloaded_app,
            (void *)(ADDR_AS_U32(__DAPP_INFO_ADDRESS) + i * FLASH_SECTOR_SIZE),
            FLASH_SECTOR_SIZE);
        if (app_info) {
            app_info = false;

            // Set download flag false and restore true
            auto *p{(uint32_t *)swap_buffer_app};
            p += 9;
            *p = FALSE_NUMBER;
            p++;
            *p = TRUE_MAGIC_NUMBER;

            // Set download and restore flag false
            p = (uint32_t *)swap_buffer_downloaded_app;
            p += 9;
            *p = FALSE_NUMBER;
            p++;
            *p = FALSE_NUMBER;
        }
        flash_range_erase(ADDR_WITH_XIP_OFFSET_AS_U32(__APP_INFO_ADDRESS) +
                              i * FLASH_SECTOR_SIZE,
                          FLASH_SECTOR_SIZE);
        flash_range_erase(ADDR_WITH_XIP_OFFSET_AS_U32(__DAPP_INFO_ADDRESS) +
                              i * FLASH_SECTOR_SIZE,
                          FLASH_SECTOR_SIZE);
        flash_range_program(ADDR_WITH_XIP_OFFSET_AS_U32(__APP_INFO_ADDRESS) +
                                i * FLASH_SECTOR_SIZE,
                            swap_buffer_downloaded_app, FLASH_SECTOR_SIZE);
        flash_range_program(ADDR_WITH_XIP_OFFSET_AS_U32(__DAPP_INFO_ADDRESS) +
                                i * FLASH_SECTOR_SIZE,
                            swap_buffer_app, FLASH_SECTOR_SIZE);
    }
    restore_interrupts(saved_interrupts);
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

    if (check_download_app_flag()) {
        puts("New app was downloaded!");
        if (verify_hash(ADDR_AS_U32(__DAPP_HASH_ADDRESS),
                        ADDR_AS_U32(__DAPP_ADDRESS),
                        ADDR_AS_U32(__DAPP_SIZE_ADDRESS))) {
            puts("New app hash was verified, swap images!");
            swap_app_images();
        } else {
            puts("New app hash verification FAILED!");
        }
    }

    if (verify_hash(ADDR_AS_U32(__APP_HASH_ADDRESS), ADDR_AS_U32(__APP_ADDRESS),
                    ADDR_AS_U32(__APP_SIZE_ADDRESS))) {
        jump_to_vtor(ADDR_AS_U32(__APP_ADDRESS));
    }
    puts("Hash verification failed");
    while (true) {
    }

    return 0;
}
