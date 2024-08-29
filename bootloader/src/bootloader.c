#include <stdint.h>
#include <stdio.h>

#include "RP2040.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

#define PICO_UART uart0
#define PICO_UART_BAUD_RATE PICO_DEFAULT_UART_BAUD_RATE
enum { PICO_UART_TX_PIN = 16, PICO_UART_RX_PIN = 17 };

#define BOOTLOADER_SIZE (0x8000U)
#define MAIN_APP_START_ADDRESS (XIP_BASE + BOOTLOADER_SIZE)

static void jump_to_vtor(uint32_t vtor) {
    typedef void (*funcPtr)(void);

    puts("Start app ...");

    uint32_t reset_vector = *(volatile uint32_t *) (vtor + 0x04);
    funcPtr app_main = (funcPtr) reset_vector;

    SCB->VTOR = (volatile uint32_t)(vtor);
    app_main();
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

    jump_to_vtor(MAIN_APP_START_ADDRESS);

    return 0;
}
