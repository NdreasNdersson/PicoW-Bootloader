#include "bootloader.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "RP2040.h"
#include "bootloader_lib.h"
#include "hardware/flash.h"
#include "linker_definitions.h"
#include "pico/stdlib.h"

void Bootloader::start_user_app() {
    typedef void (*funcPtr)();

    auto vtor{ADDR_AS_U32(APP_ADDRESS)};
    printf("Start app at %#X...\n", vtor);

    uint32_t reset_vector = *(volatile uint32_t *)(vtor + 0x04);
    auto app_main = (funcPtr)reset_vector;

    SCB->VTOR = (volatile uint32_t)(vtor);
    app_main();
}
