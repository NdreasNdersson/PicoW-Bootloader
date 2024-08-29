#include <stdint.h>
#include <stdio.h>

#include "RP2040.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

#define PICO_UART uart0
#define PICO_UART_BAUD_RATE PICO_DEFAULT_UART_BAUD_RATE
enum { PICO_UART_TX_PIN = 16, PICO_UART_RX_PIN = 17 };

#define BOOTLOADER_SIZE (0x8000U)
#define MAIN_APP_START_ADDRESS (XIP_BASE + BOOTLOADER_SIZE) + 0x100U

static void disable_interrupts(void) {
    SysTick->CTRL &= ~1;

    NVIC->ICER[0] = 0xFFFFFFFF;
    NVIC->ICPR[0] = 0xFFFFFFFF;
}

static void reset_peripherals(void) {
    reset_block(~(RESETS_RESET_IO_QSPI_BITS | RESETS_RESET_PADS_QSPI_BITS
                  | RESETS_RESET_SYSCFG_BITS | RESETS_RESET_PLL_SYS_BITS));
}

static void jump_to_vtor(uint32_t vtor) {
    // Derived from the Leaf Labs Cortex-M3 bootloader.
    // Copyright (c) 2010 LeafLabs LLC.
    // Modified 2021 Brian Starkey <stark3y@gmail.com>
    // Originally under The MIT License

    uint32_t reset_vector = *(volatile uint32_t *) (vtor + 0x04);
    SCB->VTOR = (volatile uint32_t)(vtor);

    asm volatile("msr msp, %0" ::"g"(*(volatile uint32_t *) vtor));
    asm volatile("bx %0" ::"r"(reset_vector));
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

    disable_interrupts();
    reset_peripherals();
    jump_to_vtor(MAIN_APP_START_ADDRESS);

    return 0;
}
