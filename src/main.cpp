#include <cassert>
#include <cstdio>

#include "RP2040.h"
#include "bootloader.h"
#include "common_definitions.h"
#include "hal/pico_interface_impl.h"
#include "linker_definitions.h"
#include "pico/stdlib.h"

static void print_welcome_message() {
    puts("");
    puts("******************************************************");
    puts("*                                                    *");
    puts("*           Raspberry Pi Pico W Bootloader           *");
    puts("*                                                    *");
    puts("******************************************************");
    puts("");
}

static void start_user_app() {
    typedef void (*funcPtr)();

    auto vtor{ADDR_AS_U32(APP_ADDRESS)};
    printf("Start app at %#X...\n", vtor);

    uint32_t reset_vector = *(volatile uint32_t *)(vtor + 0x04);
    auto app_main = (funcPtr)reset_vector;

    SCB->VTOR = (volatile uint32_t)(vtor);
    app_main();
}

auto main() -> int {
    stdio_uart_init_full(PICO_UART, PICO_UART_BAUD_RATE, PICO_UART_TX_PIN,
                         PICO_UART_RX_PIN);
    print_welcome_message();

    assert(SHA256_DIGEST_SIZE == ADDR_AS_U32(APP_HASH_LENGTH));
    assert(4 == ADDR_AS_U32(APP_INFO_FLAG_LENGTH));
    assert(4 == ADDR_AS_U32(APP_SIZE_LENGTH));

    Bootloader bootloader{new PicoInterfaceImpl()};
    if (bootloader.check_download_app_flag()) {
        puts("New app was downloaded!");
        if (bootloader.verify_swap_app_hash()) {
            puts("New app hash was verified, swap images!");
            bootloader.swap_app_images();
        } else {
            puts("New app hash verification FAILED!");
        }
    }

    if (bootloader.check_restore_at_boot()) {
        puts("Restore was requested before power off!");
        if (bootloader.verify_swap_app_hash()) {
            puts("Backed up hash was verified, swap images!");
            bootloader.swap_app_images();
        } else {
            puts("Backed up app hash verification FAILED!");
        }
    }

    if (bootloader.verify_app_hash()) {
        start_user_app();
    }
    puts("Hash verification failed");
    while (true) {
    }

    return 0;
}
