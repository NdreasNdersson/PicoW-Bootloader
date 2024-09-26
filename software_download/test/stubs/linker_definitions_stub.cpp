#include <cstdint>

#include "linker_definitions.h"
#include "software_download.h"

constexpr uint32_t APP_MEMORY_SIZE{1004 * 1024};
uint8_t app_area[APP_MEMORY_SIZE]{};
uint8_t swap_area[APP_MEMORY_SIZE]{};

app_info_t app_info{};

uint32_t APP_LENGTH{APP_MEMORY_SIZE};
uint32_t APP_ADDRESS{ADDR_AS_U32(app_area)};
uint32_t APP_SIZE_ADDRESS{
    reinterpret_cast<uint32_t>(&app_info.content.app_size)};
uint32_t SWAP_APP_ADDRESS{ADDR_AS_U32(swap_area)};
uint32_t SWAP_APP_SIZE_ADDRESS{
    reinterpret_cast<uint32_t>(&app_info.content.swap_app_size)};
uint32_t APP_INFO_ADDRESS{reinterpret_cast<uint32_t>(&app_info.raw)};

uint8_t g_app_info[FLASH_PAGE_SIZE]{};
