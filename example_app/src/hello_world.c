#include <stdio.h>

#include "common_definitions.h"
#include "pico/stdlib.h"

int main() {
    stdio_uart_init_full(PICO_UART, PICO_UART_BAUD_RATE, PICO_UART_TX_PIN,
                         PICO_UART_RX_PIN);

    puts(".. app started!");

    while (true) {
    }
}
