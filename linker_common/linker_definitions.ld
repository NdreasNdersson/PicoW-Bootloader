FLASH_START = 0x10000000;
TOTAL_FLASH = 2048K;

BOOTLOADER_LENGTH = 32K;
APP_INFO_LENGTH = 4K;
APP_STORAGE_LENGTH = 4K;
/* (TOTAL_FLASH - BOOTLOADER_LENGTH - APP_INFO_LENGTH - APP_STORAGE_LENGTH) / 2  = 1004K */
APP_LENGTH = 1004K;

APP_HASH_LENGTH = 32;
APP_SIZE_LENGTH = 4;
APP_INFO_FLAG_LENGTH = 4;

ASSERT((APP_HASH_LENGTH + APP_INFO_FLAG_LENGTH * 2) <= APP_INFO_LENGTH, "Not enought space in app info");

APP_INFO_ADDRESS = FLASH_START + BOOTLOADER_LENGTH;
APP_STORAGE_ADDRESS = APP_INFO_ADDRESS + APP_INFO_LENGTH;
APP_ADDRESS = APP_STORAGE_ADDRESS + APP_STORAGE_LENGTH;
SWAP_APP_ADDRESS = APP_ADDRESS + APP_LENGTH;

/* App info content */
APP_HASH_ADDRESS = APP_INFO_ADDRESS;
SWAP_APP_HASH_ADDRESS = APP_HASH_ADDRESS + APP_HASH_LENGTH;
APP_SIZE_ADDRESS = SWAP_APP_HASH_ADDRESS + APP_HASH_LENGTH;
SWAP_APP_SIZE_ADDRESS = APP_SIZE_ADDRESS + APP_SIZE_LENGTH;
APP_DOWNLOADED_FLAG_ADDRESS = SWAP_APP_SIZE_ADDRESS + APP_SIZE_LENGTH;
APP_BACKED_UP_FLAG_ADDRESS = APP_DOWNLOADED_FLAG_ADDRESS + APP_INFO_FLAG_LENGTH;


ASSERT((BOOTLOADER_LENGTH + APP_INFO_LENGTH + APP_STORAGE_LENGTH + APP_INFO_LENGTH * 2) <= TOTAL_FLASH, "Flash partitions defined incorrectly");
