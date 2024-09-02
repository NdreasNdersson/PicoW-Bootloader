#include <mbedtls/sha256.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "RP2040.h"
#include "common_definitions.h"
#include "linker_definitions.h"
#include "pico/stdlib.h"

constexpr std::uint8_t SHA256_DIGEST_SIZE{32};
constexpr std::uint32_t TRUE_MAGIC_NUMBER{14253};

static auto verify_hash(const uint32_t hash_address,
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

    auto image_start_address = ADDR_AS_U32(__APP_ADDRESS);
    auto image_size = (size_t) * ((std::uint32_t *)app_size_address);
    ret = mbedtls_sha256_update_ret(
        &sha256_ctx, (const unsigned char *)image_start_address, image_size);
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
    return TRUE_MAGIC_NUMBER == (*((std::uint32_t *)ADDR_AS_U32(
                                    __DOWNLOAD_APP_DOWNLOAD_FLAG_ADDRESS)));
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
    assert(4 == __APP_SIZE_LENGTH);
    assert(4 == __APP_DOWNLOAD_FLAG_LENGTH);

    if (check_download_app_flag()) {
        puts("New app was downloaded!");
        if (verify_hash(ADDR_AS_U32(__DOWNLOAD_APP_HASH_ADDRESS),
                        ADDR_AS_U32(__DOWNLOAD_APP_SIZE_ADDRESS))) {
            puts("New app hash was verified!");
        } else {
            puts("New app hash verification FAILED!");
        }
    }

    if (verify_hash(ADDR_AS_U32(__APP_HASH_ADDRESS),
                    ADDR_AS_U32(__APP_SIZE_ADDRESS))) {
        jump_to_vtor(ADDR_AS_U32(__APP_ADDRESS));
    }
    puts("Hash verification failed");
    while (true) {
    }

    return 0;
}
