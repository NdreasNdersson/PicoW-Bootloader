#ifndef PICO_BOOTLOADER_RP2040_STUB_H
#define PICO_BOOTLOADER_RP2040_STUB_H

#include <cstdint>

typedef struct {
    uint32_t VTOR;
} SCB_Type;

SCB_Type scb_type{};
SCB_Type *SCB{&scb_type};

#endif  // PICO_BOOTLOADER_RP2040_STUB_H
