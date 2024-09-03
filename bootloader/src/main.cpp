#include <cassert>
#include <cstdio>

#include "bootloader.h"
#include "common_definitions.h"
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

auto main() -> int {
    stdio_uart_init_full(PICO_UART, PICO_UART_BAUD_RATE, PICO_UART_TX_PIN,
                         PICO_UART_RX_PIN);
    print_welcome_message();
    sleep_ms(1000);

    assert(SHA256_DIGEST_SIZE == ADDR_AS_U32(__APP_HASH_LENGTH));
    assert(4 == ADDR_AS_U32(__APP_INFO_FLAG_LENGTH));
    assert(4 == ADDR_AS_U32(__APP_SIZE_LENGTH));

    auto bootloader = Bootloader();
    if (bootloader.check_download_app_flag()) {
        puts("New app was downloaded!");
        if (bootloader.verify_swap_app_hash()) {
            puts("New app hash was verified, swap images!");
            bootloader.swap_app_images();
        } else {
            puts("New app hash verification FAILED!");
        }
    }

    if (bootloader.verify_app_hash()) {
        bootloader.start_user_app();
    }
    puts("Hash verification failed");
    while (true) {
    }

    return 0;
}
