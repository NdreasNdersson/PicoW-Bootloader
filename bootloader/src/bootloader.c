#include <stdint.h>
#include <stdio.h>

#include "RP2040.h"
#include "pico/stdlib.h"

#define PICO_UART uart0
#define PICO_UART_BAUD_RATE PICO_DEFAULT_UART_BAUD_RATE
enum { PICO_UART_TX_PIN = 16, PICO_UART_RX_PIN = 17 };

#define BOOTLOADER_SIZE (0x8000U)
#define MAIN_APP_START_ADDRESS (XIP_BASE + BOOTLOADER_SIZE)

static void jump_to_main(void) {
    puts("Start app in 1s ...");
    sleep_ms(1000);
    stdio_uart_deinit();

    typedef void (*void_fn)(void);
    uint32_t* reset_vector_entry = (uint32_t*)(MAIN_APP_START_ADDRESS + 4U);

    uint32_t* reset_vector = (uint32_t*)(*reset_vector_entry);
    void_fn jump_fn = (void_fn)reset_vector;

    SCB->VTOR = MAIN_APP_START_ADDRESS;
    jump_fn();
}

static void print_welcome_message(void) {
    puts("");
    puts("******************************************************");
    puts("*                                                    *");
    puts("*           Raspberry Pi Pico W Bootloader           *");
    puts("*                                                    *");
    puts("******************************************************");
    puts("");
}

int main(void) {
    stdio_uart_init_full(PICO_UART, PICO_UART_BAUD_RATE, PICO_UART_TX_PIN,
                         PICO_UART_RX_PIN);
    print_welcome_message();
    sleep_ms(1000);
    jump_to_main();

    return 0;
}
