#include <stdint.h>
#include <stdio.h>

#include "RP2040.h"
#include "common_definitions.h"
#include "linker_definitions.h"
#include "pico/stdlib.h"

static void jump_to_vtor(uint32_t vtor) {
    typedef void (*funcPtr)(void);

    printf("Start app at %#X...", vtor);

    uint32_t reset_vector = *(volatile uint32_t *)(vtor + 0x04);
    funcPtr app_main = (funcPtr)reset_vector;

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

    jump_to_vtor(ADDR_AS_U32(__FLASH_APP_START));

    return 0;
}
