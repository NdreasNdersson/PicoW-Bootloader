#include "pico_interface_impl.h"

#include <cstdio>

#include "hardware/watchdog.h"
#include "mbedtls/sha256.h"
#include "pico/error.h"
#include "pico/flash.h"
#include "types.h"

constexpr uint32_t ENTER_EXIT_TIMEOUT_MS{1000U};

void PicoInterfaceImpl::watchdog_enable(uint32_t delay_ms,
                                        bool pause_on_debug) {
    watchdog_enable(delay_ms, pause_on_debug);
}

auto PicoInterfaceImpl::store_to_flash(uint32_t flash_offs, const uint8_t *data,
                                       size_t count) -> bool {
    store_to_flash_t store_data{flash_offs, data, count};
    return PICO_OK == flash_safe_execute(&program,
                                         reinterpret_cast<void *>(&store_data),
                                         ENTER_EXIT_TIMEOUT_MS);
}

auto PicoInterfaceImpl::erase_flash(uint32_t flash_offs, size_t count) -> bool {
    erase_flash_t erase_data{flash_offs, count};
    return PICO_OK == flash_safe_execute(&erase,
                                         reinterpret_cast<void *>(&erase_data),
                                         ENTER_EXIT_TIMEOUT_MS);
}

auto PicoInterfaceImpl::verify_hash(
    const unsigned char stored_sha256[SHA256_DIGEST_SIZE],
    const uint32_t app_address, const uint32_t app_size) -> bool {
    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);

    int ret;
    ret = mbedtls_sha256_starts_ret(&sha256_ctx, 0);
    if (ret) {
        return false;
    }

    ret = mbedtls_sha256_update_ret(
        &sha256_ctx, (const unsigned char *)app_address, app_size);
    if (ret) {
        return false;
    }

    unsigned char calculated_sha256[SHA256_DIGEST_SIZE]{};
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

            printf("App size: %u\n", app_size);
            printf("Stored hash:\n");
            for (size_t i{0}; i < SHA256_DIGEST_SIZE; i++) {
                printf("%x", stored_sha256[i]);
            }
            puts("");

            printf("Calculated hash:\n");
            for (size_t i{0}; i < SHA256_DIGEST_SIZE; i++) {
                printf("%x", calculated_sha256[i]);
            }
            puts("");
            break;
        }
    }

    return hash_matched;
}

void PicoInterfaceImpl::program(void *data) {
    auto store_to_flash_data{reinterpret_cast<store_to_flash_t *>(data)};
    flash_range_program(store_to_flash_data->flash_offs,
                        store_to_flash_data->data, store_to_flash_data->count);
}

void PicoInterfaceImpl::erase(void *data) {
    auto erase_data{reinterpret_cast<erase_flash_t *>(data)};
    flash_range_erase(erase_data->flash_offs, erase_data->count);
}
