#include "bootloader.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstring>
#include <iterator>
#include <memory>

#include "gmock/gmock.h"
#include "hardware/flash.h"
#include "linker_definitions.h"
#include "mocks/mock_bootloader_lib.h"
#include "pico_interface.h"
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

TEST_F(BootloaderTest, InitDownload) {
    const uint32_t size{FLASH_SECTOR_SIZE * 2 + 100};

    testing::StrictMock<MockPicoInterface> mock_pico_interface;
    std::unique_ptr<Bootloader> uut{
        std::make_unique<SoftwareDownload>(mock_pico_interface)};

    {
        EXPECT_CALL(mock_pico_interface,
                    erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(false));
        EXPECT_EQ(uut->init_download(size), false);
    }

    {
        app_info_t actual_app_info{};
        auto copy_app_info = [&actual_app_info](uint32_t arg0,
                                                const uint8_t *arg1,
                                                size_t arg2) {
            std::memcpy(actual_app_info.raw, arg1, FLASH_PAGE_SIZE);
        };
        EXPECT_CALL(mock_pico_interface,
                    erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(mock_pico_interface,
                    store_to_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::DoAll(testing::Invoke(copy_app_info),
                                     testing::Return(false)));
        EXPECT_EQ(uut->init_download(size), false);
        EXPECT_EQ(actual_app_info.content.swap_app_size, size);
        EXPECT_EQ(actual_app_info.content.app_backed_up, FALSE_NUMBER);
        EXPECT_EQ(actual_app_info.content.app_downloaded, FALSE_NUMBER);
    }

    {
        EXPECT_CALL(mock_pico_interface,
                    erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(mock_pico_interface,
                    store_to_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(mock_pico_interface,
                    erase_flash(ADDR_AS_U32(SWAP_APP_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(mock_pico_interface,
                    erase_flash(ADDR_AS_U32(SWAP_APP_ADDRESS) - XIP_BASE +
                                    FLASH_SECTOR_SIZE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(false));
        EXPECT_EQ(uut->init_download(size), false);
    }

    {
        testing::InSequence s;
        EXPECT_CALL(mock_pico_interface,
                    erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(mock_pico_interface,
                    store_to_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(mock_pico_interface,
                    erase_flash(testing::_, FLASH_SECTOR_SIZE))
            .Times(3)
            .WillRepeatedly(testing::Return(true));
        EXPECT_EQ(uut->init_download(size), true);
    }
}

TEST_F(BootloaderTest, SetHash) {
    testing::StrictMock<MockPicoInterface> mock_pico_interface;
    std::unique_ptr<Bootloader> uut{
        std::make_unique<SoftwareDownload>(mock_pico_interface)};

    const unsigned char expected_hash[SHA256_DIGEST_SIZE]{'H', 'A', 'S', 'H'};
    app_info_t actual_app_info{};
    auto copy_app_info = [&actual_app_info](uint32_t arg0, const uint8_t *arg1,
                                            size_t arg2) {
        std::memcpy(actual_app_info.raw, arg1, FLASH_PAGE_SIZE);
    };
    EXPECT_CALL(mock_pico_interface,
                erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                            FLASH_SECTOR_SIZE))
        .WillOnce(testing::Return(true));
    EXPECT_CALL(mock_pico_interface,
                store_to_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                               testing::_, FLASH_PAGE_SIZE))
        .WillOnce(testing::DoAll(testing::Invoke(copy_app_info),
                                 testing::Return(true)));
    EXPECT_TRUE(uut->set_hash(expected_hash));

    for (int i = 0; i < SHA256_DIGEST_SIZE; ++i) {
        EXPECT_EQ(actual_app_info.content.swap_app_hash[i], expected_hash[i])
            << "Hash differ at index " << i;
    }
}

TEST_F(BootloaderTest, WriteApp) {
    testing::StrictMock<MockPicoInterface> mock_pico_interface;
    std::unique_ptr<Bootloader> uut{
        std::make_unique<SoftwareDownload>(mock_pico_interface)};

    unsigned char actual_binary_block[FLASH_PAGE_SIZE]{};
    auto copy_binary_block = [&actual_binary_block](uint32_t arg0,
                                                    const uint8_t *arg1,
                                                    size_t arg2) {
        std::memcpy(actual_binary_block, arg1, FLASH_PAGE_SIZE);
    };

    app_info_t app_info{};
    app_info.content.swap_app_size = FLASH_PAGE_SIZE + FLASH_PAGE_SIZE / 2;
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);

    unsigned char binary_block[FLASH_PAGE_SIZE]{'H', 'A', 'S', 'H'};

    {
        EXPECT_CALL(mock_pico_interface,
                    store_to_flash(ADDR_AS_U32(SWAP_APP_ADDRESS) - XIP_BASE,
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::Return(false));
        EXPECT_FALSE(uut->write_app(binary_block));
    }

    {
        EXPECT_CALL(mock_pico_interface,
                    store_to_flash(ADDR_AS_U32(SWAP_APP_ADDRESS) - XIP_BASE,
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::DoAll(testing::Invoke(copy_binary_block),
                                     testing::Return(true)));
        EXPECT_TRUE(uut->write_app(binary_block));
        for (int i = 0; i < SHA256_DIGEST_SIZE; ++i) {
            EXPECT_EQ(actual_binary_block[i], binary_block[i])
                << "Binary vector differ at index " << i;
        }

        binary_block[10] = 'A';
        EXPECT_CALL(mock_pico_interface,
                    store_to_flash(ADDR_AS_U32(SWAP_APP_ADDRESS) - XIP_BASE +
                                       FLASH_PAGE_SIZE,
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::DoAll(testing::Invoke(copy_binary_block),
                                     testing::Return(true)));
        EXPECT_TRUE(uut->write_app(binary_block));
        for (int i = 0; i < FLASH_PAGE_SIZE; ++i) {
            EXPECT_EQ(actual_binary_block[i], binary_block[i])
                << "Binary vector differ at index " << i;
        }
    }
    { EXPECT_FALSE(uut->write_app(binary_block)); }
}

TEST_F(BootloaderTest, DownloadComplete) {
    testing::StrictMock<MockPicoInterface> mock_pico_interface;
    std::unique_ptr<Bootloader> uut{
        std::make_unique<SoftwareDownload>(mock_pico_interface)};

    {
        EXPECT_CALL(mock_pico_interface,
                    erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(false));
        EXPECT_FALSE(uut->download_complete());
    }

    {
        EXPECT_CALL(mock_pico_interface,
                    erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(mock_pico_interface,
                    store_to_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(mock_pico_interface,
                    verify_hash(testing::_, ADDR_AS_U32(SWAP_APP_ADDRESS),
                                SHA256_DIGEST_SIZE))
            .WillOnce(testing::Return(false));

        EXPECT_FALSE(uut->download_complete());
    }

    {
        unsigned char actual_hash[SHA256_DIGEST_SIZE]{'H', 'A', 'S', 'H'};
        auto copy_hash = [&actual_hash](
                             const unsigned char arg0[SHA256_DIGEST_SIZE],
                             const uint32_t arg1, const uint32_t arg2) {
            std::memcpy(actual_hash, arg0, SHA256_DIGEST_SIZE);
        };

        app_info_t app_info{};
        std::memcpy(app_info.content.swap_app_hash, actual_hash,
                    SHA256_DIGEST_SIZE);
        std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);

        EXPECT_CALL(mock_pico_interface,
                    erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(mock_pico_interface,
                    store_to_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(mock_pico_interface,
                    verify_hash(testing::_, ADDR_AS_U32(SWAP_APP_ADDRESS),
                                SHA256_DIGEST_SIZE))
            .WillOnce(testing::DoAll(testing::Invoke(copy_hash),
                                     testing::Return(true)));
        EXPECT_CALL(mock_pico_interface, watchdog_enable(testing::_, true));

        EXPECT_TRUE(uut->download_complete());

        for (int i = 0; i < SHA256_DIGEST_SIZE; ++i) {
            EXPECT_EQ(actual_hash[i], app_info.content.swap_app_hash[i])
                << "Hash differ at index " << i;
        }
    }
}

TEST_F(BootloaderTest, VerifyAppHash) {
    testing::StrictMock<MockPicoInterface> mock_pico_interface;
    std::unique_ptr<Bootloader> uut{
        std::make_unique<SoftwareDownload>(mock_pico_interface)};

    {
        EXPECT_CALL(mock_pico_interface,
                    verify_hash(testing::_, ADDR_AS_U32(APP_ADDRESS),
                                SHA256_DIGEST_SIZE))
            .WillOnce(testing::Return(false));

        EXPECT_FALSE(uut->verify_app_hash());
    }

    {
        unsigned char actual_hash[SHA256_DIGEST_SIZE]{'H', 'A', 'S', 'H'};
        auto copy_hash = [&actual_hash](
                             const unsigned char arg0[SHA256_DIGEST_SIZE],
                             const uint32_t arg1, const uint32_t arg2) {
            std::memcpy(actual_hash, arg0, SHA256_DIGEST_SIZE);
        };

        app_info_t app_info{};
        std::memcpy(app_info.content.app_hash, actual_hash, SHA256_DIGEST_SIZE);
        std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);

        EXPECT_CALL(mock_pico_interface,
                    verify_hash(testing::_, ADDR_AS_U32(APP_ADDRESS),
                                SHA256_DIGEST_SIZE))
            .WillOnce(testing::DoAll(testing::Invoke(copy_hash),
                                     testing::Return(true)));

        EXPECT_TRUE(uut->verify_app_hash());

        for (int i = 0; i < SHA256_DIGEST_SIZE; ++i) {
            EXPECT_EQ(actual_hash[i], app_info.content.app_hash[i])
                << "Hash differ at index " << i;
        }
    }
}

TEST_F(BootloaderTest, VerifySwapAppHash) {
    testing::StrictMock<MockPicoInterface> mock_pico_interface;
    std::unique_ptr<Bootloader> uut{
        std::make_unique<SoftwareDownload>(mock_pico_interface)};

    {
        EXPECT_CALL(mock_pico_interface,
                    verify_hash(testing::_, ADDR_AS_U32(SWAP_APP_ADDRESS),
                                SHA256_DIGEST_SIZE))
            .WillOnce(testing::Return(false));

        EXPECT_FALSE(uut->verify_swap_app_hash());
    }

    {
        unsigned char actual_hash[SHA256_DIGEST_SIZE]{'H', 'A', 'S', 'H'};
        auto copy_hash = [&actual_hash](
                             const unsigned char arg0[SHA256_DIGEST_SIZE],
                             const uint32_t arg1, const uint32_t arg2) {
            std::memcpy(actual_hash, arg0, SHA256_DIGEST_SIZE);
        };

        app_info_t app_info{};
        std::memcpy(app_info.content.swap_app_hash, actual_hash,
                    SHA256_DIGEST_SIZE);
        std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);

        EXPECT_CALL(mock_pico_interface,
                    verify_hash(testing::_, ADDR_AS_U32(SWAP_APP_ADDRESS),
                                SHA256_DIGEST_SIZE))
            .WillOnce(testing::DoAll(testing::Invoke(copy_hash),
                                     testing::Return(true)));

        EXPECT_TRUE(uut->verify_swap_app_hash());

        for (int i = 0; i < SHA256_DIGEST_SIZE; ++i) {
            EXPECT_EQ(actual_hash[i], app_info.content.swap_app_hash[i])
                << "Hash differ at index " << i;
        }
    }
}

TEST_F(BootloaderTest, CheckDownloadAppFlag) {
    testing::StrictMock<MockPicoInterface> mock_pico_interface;
    std::unique_ptr<Bootloader> uut{
        std::make_unique<SoftwareDownload>(mock_pico_interface)};

    EXPECT_EQ(uut->check_download_app_flag(), false);

    app_info_t app_info{};
    app_info.content.app_downloaded = TRUE_MAGIC_NUMBER;
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);
    EXPECT_EQ(uut->check_download_app_flag(), true);

    app_info.content.app_downloaded = true;
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);
    EXPECT_EQ(uut->check_download_app_flag(), false);
}

TEST_F(BootloaderTest, CheckRestoreAtBoot) {
    testing::StrictMock<MockPicoInterface> mock_pico_interface;
    std::unique_ptr<Bootloader> uut{
        std::make_unique<SoftwareDownload>(mock_pico_interface)};

    EXPECT_EQ(uut->check_restore_at_boot(), false);

    app_info_t app_info{};
    app_info.content.app_backed_up = TRUE_MAGIC_NUMBER;
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);
    EXPECT_EQ(uut->check_restore_at_boot(), false);

    app_info.content.app_restore_at_boot = TRUE_MAGIC_NUMBER;
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);
    EXPECT_EQ(uut->check_restore_at_boot(), true);

    app_info.content.app_backed_up = FALSE_NUMBER;
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);
    EXPECT_EQ(uut->check_restore_at_boot(), false);
}
