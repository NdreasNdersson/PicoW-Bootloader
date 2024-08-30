#ifndef PICOW_BOOTLOADER_LINKER_DEFINITIONS_H
#define PICOW_BOOTLOADER_LINKER_DEFINITIONS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ADDR_AS_U32(Data) (uint32_t) & (Data)

extern uint32_t __APP_HASH_ADDRESS;
extern uint32_t __APP_HASH_LENGTH;
extern uint32_t __APP_SIZE_ADDRESS;
extern uint32_t __APP_SIZE_LENGTH;
extern uint32_t __APP_ADDRESS;

#ifdef __cplusplus
}
#endif

#endif
