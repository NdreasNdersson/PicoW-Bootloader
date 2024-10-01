#include "bootloader.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstring>

#include "gmock/gmock.h"
#include "hardware/flash.h"
#include "linker_definitions.h"
#include "mocks/mock_pico_interface.h"

namespace PicoBootloader {

class BootloaderTest : public testing::Test {
   protected:
    BootloaderTest()
        : mock_pico_interface_{new testing::StrictMock<MockPicoInterface>()},
          uut_{mock_pico_interface_} {
        testing::Mock::AllowLeak(mock_pico_interface_);
    }

    testing::StrictMock<MockPicoInterface> *mock_pico_interface_;
    Bootloader uut_;
};

TEST_F(BootloaderTest, CheckDownloadAppFlag) {
    EXPECT_EQ(uut_.check_download_app_flag(), false);

    app_info_t app_info{};
    app_info.content.app_downloaded = TRUE_MAGIC_NUMBER;
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);
    EXPECT_EQ(uut_.check_download_app_flag(), true);

    app_info.content.app_downloaded = true;
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);
    EXPECT_EQ(uut_.check_download_app_flag(), false);
}

TEST_F(BootloaderTest, CheckRestoreAtBoot) {
    EXPECT_EQ(uut_.check_restore_at_boot(), false);

    app_info_t app_info{};
    app_info.content.app_backed_up = TRUE_MAGIC_NUMBER;
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);
    EXPECT_EQ(uut_.check_restore_at_boot(), false);

    app_info.content.app_restore_at_boot = TRUE_MAGIC_NUMBER;
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);
    EXPECT_EQ(uut_.check_restore_at_boot(), true);

    app_info.content.app_backed_up = FALSE_NUMBER;
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);
    EXPECT_EQ(uut_.check_restore_at_boot(), false);
}

TEST_F(BootloaderTest, SwapAppImages) {
    app_info_t app_info{};
    const auto app_size{FLASH_SECTOR_SIZE};
    const auto swap_app_size{FLASH_SECTOR_SIZE * 2 + FLASH_PAGE_SIZE};
    const unsigned char app_hash[SHA256_DIGEST_SIZE]{'A', 'P', 'P', 'H',
                                                     'A', 'S', 'H'};
    const unsigned char swap_app_hash[SHA256_DIGEST_SIZE]{'S', 'W', 'A', 'P',
                                                          'H', 'A', 'S', 'H'};
    app_info.content.app_size = app_size;
    app_info.content.swap_app_size = swap_app_size;
    std::memcpy(app_info.content.app_hash, app_hash, SHA256_DIGEST_SIZE);
    std::memcpy(app_info.content.swap_app_hash, swap_app_hash,
                SHA256_DIGEST_SIZE);
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);

    EXPECT_CALL(*mock_pico_interface_,
                erase_flash(testing::_, FLASH_SECTOR_SIZE))
        .Times(6)
        .WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*mock_pico_interface_,
                store_to_flash(testing::_, testing::_, FLASH_SECTOR_SIZE))
        .Times(6)
        .WillRepeatedly(testing::Return(true));

    app_info_t actual_app_info{};
    auto copy_app_info = [&actual_app_info](uint32_t arg0, const uint8_t *arg1,
                                            size_t arg2) {
        std::memcpy(actual_app_info.raw, arg1, FLASH_PAGE_SIZE);
    };
    EXPECT_CALL(*mock_pico_interface_,
                erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                            FLASH_SECTOR_SIZE))
        .WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_pico_interface_,
                store_to_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                               testing::_, FLASH_PAGE_SIZE))
        .WillOnce(testing::DoAll(testing::Invoke(copy_app_info),
                                 testing::Return(true)));

    uut_.swap_app_images();
    EXPECT_EQ(actual_app_info.content.app_size, swap_app_size);
    EXPECT_EQ(actual_app_info.content.swap_app_size, app_size);
    EXPECT_EQ(actual_app_info.content.app_backed_up, TRUE_MAGIC_NUMBER);
    EXPECT_EQ(actual_app_info.content.app_downloaded, FALSE_NUMBER);
    EXPECT_EQ(actual_app_info.content.app_restore_at_boot, FALSE_NUMBER);

    for (int i = 0; i < SHA256_DIGEST_SIZE; ++i) {
        EXPECT_EQ(actual_app_info.content.swap_app_hash[i], app_hash[i])
            << "Hash differ at index " << i;
        EXPECT_EQ(actual_app_info.content.app_hash[i], swap_app_hash[i])
            << "Hash differ at index " << i;
    }
}

}  // namespace PicoBootloader
