#ifndef PICOW_BOOTLOADER_LINKER_DEFINITIONS_H
#define PICOW_BOOTLOADER_LINKER_DEFINITIONS_H

#include <cstdint>

#include "hardware/regs/addressmap.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ADDR_AS_U32(Data) (uint32_t) & (Data)
#define ADDR_WITH_XIP_OFFSET_AS_U32(Data) ADDR_AS_U32(Data) - XIP_BASE

extern uint32_t __APP_INFO_LENGTH;
extern uint32_t __APP_STORAGE_LENGTH;
extern uint32_t __APP_LENGTH;

extern uint32_t __APP_INFO_ADDRESS;
extern uint32_t __APP_STORAGE_ADDRESS;
extern uint32_t __APP_ADDRESS;
extern uint32_t __SWAP_APP_ADDRESS;

// App info content
extern uint32_t __APP_HASH_LENGTH;
extern uint32_t __APP_SIZE_LENGTH;
extern uint32_t __APP_INFO_FLAG_LENGTH;

extern uint32_t __APP_HASH_ADDRESS;
extern uint32_t __SWAP_APP_HASH_ADDRESS;
extern uint32_t __APP_SIZE_ADDRESS;
extern uint32_t __SWAP_APP_SIZE_ADDRESS;
extern uint32_t __APP_DOWNLOADED_FLAG_ADDRESS;
extern uint32_t __APP_BACKED_UP_FLAG_ADDRESS;

#ifdef __cplusplus
}
#endif

#endif
