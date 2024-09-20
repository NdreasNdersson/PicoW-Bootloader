#include <gmock/gmock.h>

#include <cstdint>

#include "bootloader_lib.h"

class MockBootloaderLib : public BootloaderLib {
   public:
    MOCK_METHOD(void, init_download, (const uint32_t &size), (override));
    MOCK_METHOD(void, set_hash,
                (const unsigned char app_hash[SHA256_DIGEST_SIZE]), (override));
    MOCK_METHOD(bool, write_app,
                (const unsigned char binary_block[FLASH_PAGE_SIZE]),
                (override));
    MOCK_METHOD(void, download_complete, (), (override));
    MOCK_METHOD(bool, verify_app_hash, (), (override));
    MOCK_METHOD(bool, verify_swap_app_hash, (), (override));
    MOCK_METHOD(void, reboot, (uint32_t delay), (override));
    MOCK_METHOD(bool, restore, (uint32_t delay), (override));
    MOCK_METHOD(bool, check_download_app_flag, (), (const, override));
    MOCK_METHOD(bool, check_restore_at_boot, (), (const, override));
    MOCK_METHOD(void, swap_app_images, (), (override));
};
