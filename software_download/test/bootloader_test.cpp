#include "bootloader.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstring>
#include <memory>

#include "linker_definitions.h"
#include "mocks/mock_bootloader_lib.h"
#include "software_download.h"

constexpr uint32_t EXPECTED_XIP_BASE{0x10};
constexpr uint32_t EXPECTED_FLASH_SECTOR_SIZE{1u << 12};
constexpr uint32_t EXPECTED_FLASH_PAGE_SIZE{1u << 8};

class BootloaderTest : public testing::Test {
   protected:
    // You can remove any or all of the following functions if their bodies would
    // be empty.

    BootloaderTest() {
        // You can do set-up work for each test here.
    }

    ~BootloaderTest() override {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }

    // Class members declared here can be used by all tests in the test suite
    // for Foo.
};

TEST_F(BootloaderTest, InitDownload) {
    MockPicoInterface mock_pico_interface;
    std::unique_ptr<Bootloader> uut{
        std::make_unique<SoftwareDownload>(mock_pico_interface)};

    EXPECT_CALL(mock_pico_interface,
                erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - EXPECTED_XIP_BASE,
                            EXPECTED_FLASH_SECTOR_SIZE))
        .WillOnce(testing::Return(false));
    uint32_t size{1234};
    EXPECT_EQ(uut->init_download(size), false);

    EXPECT_CALL(mock_pico_interface,
                erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - EXPECTED_XIP_BASE,
                            EXPECTED_FLASH_SECTOR_SIZE))
        .WillOnce(testing::Return(true));
    app_info_t actual_app_info{};

    auto copy_app_info = [&actual_app_info](uint32_t arg0, const uint8_t *arg1,
                                            size_t arg2) {
        std::memcpy(actual_app_info.raw, arg1, FLASH_PAGE_SIZE);
    };
    EXPECT_CALL(
        mock_pico_interface,
        store_to_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - EXPECTED_XIP_BASE,
                       testing::_, EXPECTED_FLASH_PAGE_SIZE))
        .WillOnce(testing::DoAll(testing::Invoke(copy_app_info),
                                 testing::Return(false)));
    EXPECT_EQ(uut->init_download(size), false);
    EXPECT_EQ(actual_app_info.content.swap_app_size, size);
}

TEST_F(BootloaderTest, CheckDownloadAppFlag) {
    MockPicoInterface mock_pico_interface;
    std::unique_ptr<Bootloader> uut{
        std::make_unique<SoftwareDownload>(mock_pico_interface)};

    app_info_t app_info{};
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);
    EXPECT_EQ(uut->check_download_app_flag(), false);

    app_info.content.app_downloaded = TRUE_MAGIC_NUMBER;
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);
    EXPECT_EQ(uut->check_download_app_flag(), true);

    app_info.content.app_downloaded = true;
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);
    EXPECT_EQ(uut->check_download_app_flag(), false);
}
