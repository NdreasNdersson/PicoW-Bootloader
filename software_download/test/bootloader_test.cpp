#include "bootloader.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>
#include <memory>

#include "linker_definitions.h"
#include "mocks/mock_bootloader_lib.h"
#include "software_download.h"

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
