#include "software_download.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstring>

#include "gmock/gmock.h"
#include "hardware/flash.h"
#include "linker_definitions.h"
#include "mocks/mock_pico_interface.h"

namespace PicoBootloader {

class SoftwareDownloadTest : public testing::Test {
   protected:
    SoftwareDownloadTest()
        : mock_pico_interface_{new testing::StrictMock<MockPicoInterface>()},
          uut_{mock_pico_interface_} {
        testing::Mock::AllowLeak(mock_pico_interface_);
    }

    testing::StrictMock<MockPicoInterface> *mock_pico_interface_;
    SoftwareDownload uut_;
};

TEST_F(SoftwareDownloadTest, InitDownload) {
    const uint32_t size{FLASH_SECTOR_SIZE * 2 + 100};
    {
        EXPECT_CALL(*mock_pico_interface_,
                    erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(false));
        EXPECT_EQ(uut_.init_download(size), false);
    }

    {
        app_info_t actual_app_info{};
        auto copy_app_info = [&actual_app_info](uint32_t arg0,
                                                const uint8_t *arg1,
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
                                     testing::Return(false)));
        EXPECT_EQ(uut_.init_download(size), false);
        EXPECT_EQ(actual_app_info.content.swap_app_size, size);
        EXPECT_EQ(actual_app_info.content.app_backed_up, FALSE_NUMBER);
        EXPECT_EQ(actual_app_info.content.app_downloaded, FALSE_NUMBER);
    }

    {
        EXPECT_CALL(*mock_pico_interface_,
                    erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(*mock_pico_interface_,
                    store_to_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_EQ(uut_.init_download(size), true);
    }
}

TEST_F(SoftwareDownloadTest, SetHash) {
    const unsigned char expected_hash[SHA256_DIGEST_SIZE]{'H', 'A', 'S', 'H'};
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
    EXPECT_TRUE(uut_.set_hash(expected_hash));

    for (int i = 0; i < SHA256_DIGEST_SIZE; ++i) {
        EXPECT_EQ(actual_app_info.content.swap_app_hash[i], expected_hash[i])
            << "Hash differ at index " << i;
    }
}

TEST_F(SoftwareDownloadTest, WriteApp) {
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

    testing::InSequence s;
    {
        EXPECT_CALL(*mock_pico_interface_,
                    erase_flash(ADDR_AS_U32(SWAP_APP_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(false));
        EXPECT_FALSE(uut_.write_app(binary_block));
    }

    {
        EXPECT_CALL(*mock_pico_interface_,
                    erase_flash(ADDR_AS_U32(SWAP_APP_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(*mock_pico_interface_,
                    store_to_flash(ADDR_AS_U32(SWAP_APP_ADDRESS) - XIP_BASE,
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::Return(false));
        EXPECT_FALSE(uut_.write_app(binary_block));
    }

    {
        EXPECT_CALL(*mock_pico_interface_,
                    store_to_flash(ADDR_AS_U32(SWAP_APP_ADDRESS) - XIP_BASE,
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::DoAll(testing::Invoke(copy_binary_block),
                                     testing::Return(true)));
        EXPECT_TRUE(uut_.write_app(binary_block));
        for (int i = 0; i < SHA256_DIGEST_SIZE; ++i) {
            EXPECT_EQ(actual_binary_block[i], binary_block[i])
                << "Binary vector differ at index " << i;
        }

        binary_block[10] = 'A';
        EXPECT_CALL(*mock_pico_interface_,
                    store_to_flash(ADDR_AS_U32(SWAP_APP_ADDRESS) - XIP_BASE +
                                       FLASH_PAGE_SIZE,
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::DoAll(testing::Invoke(copy_binary_block),
                                     testing::Return(true)));
        EXPECT_TRUE(uut_.write_app(binary_block));
        for (int i = 0; i < FLASH_PAGE_SIZE; ++i) {
            EXPECT_EQ(actual_binary_block[i], binary_block[i])
                << "Binary vector differ at index " << i;
        }
    }
    { EXPECT_FALSE(uut_.write_app(binary_block)); }
}

TEST_F(SoftwareDownloadTest, DownloadComplete) {
    {
        EXPECT_CALL(*mock_pico_interface_,
                    erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(false));
        EXPECT_FALSE(uut_.download_complete());
    }

    {
        EXPECT_CALL(*mock_pico_interface_,
                    erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(*mock_pico_interface_,
                    store_to_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(*mock_pico_interface_,
                    verify_hash(testing::_, ADDR_AS_U32(SWAP_APP_ADDRESS), 0))
            .WillOnce(testing::Return(false));

        EXPECT_FALSE(uut_.download_complete());
    }

    {
        unsigned char actual_hash[SHA256_DIGEST_SIZE]{'H', 'A', 'S', 'H'};
        auto copy_hash = [&actual_hash](
                             const unsigned char arg0[SHA256_DIGEST_SIZE],
                             const uint32_t arg1, const uint32_t arg2) {
            std::memcpy(actual_hash, arg0, SHA256_DIGEST_SIZE);
        };

        app_info_t app_info{};
        app_info.content.swap_app_size = __LINE__;
        std::memcpy(app_info.content.swap_app_hash, actual_hash,
                    SHA256_DIGEST_SIZE);
        std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);

        EXPECT_CALL(*mock_pico_interface_,
                    erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(*mock_pico_interface_,
                    store_to_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(*mock_pico_interface_,
                    verify_hash(testing::_, ADDR_AS_U32(SWAP_APP_ADDRESS),
                                app_info.content.swap_app_size))
            .WillOnce(testing::DoAll(testing::Invoke(copy_hash),
                                     testing::Return(true)));
        EXPECT_CALL(*mock_pico_interface_, reboot(testing::_));

        EXPECT_TRUE(uut_.download_complete());

        for (int i = 0; i < SHA256_DIGEST_SIZE; ++i) {
            EXPECT_EQ(actual_hash[i], app_info.content.swap_app_hash[i])
                << "Hash differ at index " << i;
        }
    }
}

TEST_F(SoftwareDownloadTest, VerifyAppHash) {
    {
        EXPECT_CALL(*mock_pico_interface_,
                    verify_hash(testing::_, ADDR_AS_U32(APP_ADDRESS), 0))
            .WillOnce(testing::Return(false));

        EXPECT_FALSE(uut_.verify_app_hash());
    }

    {
        unsigned char actual_hash[SHA256_DIGEST_SIZE]{'H', 'A', 'S', 'H'};
        auto copy_hash = [&actual_hash](
                             const unsigned char arg0[SHA256_DIGEST_SIZE],
                             const uint32_t arg1, const uint32_t arg2) {
            std::memcpy(actual_hash, arg0, SHA256_DIGEST_SIZE);
        };

        app_info_t app_info{};
        app_info.content.app_size = __LINE__;
        std::memcpy(app_info.content.app_hash, actual_hash, SHA256_DIGEST_SIZE);
        std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);

        EXPECT_CALL(*mock_pico_interface_,
                    verify_hash(testing::_, ADDR_AS_U32(APP_ADDRESS),
                                app_info.content.app_size))
            .WillOnce(testing::DoAll(testing::Invoke(copy_hash),
                                     testing::Return(true)));

        EXPECT_TRUE(uut_.verify_app_hash());

        for (int i = 0; i < SHA256_DIGEST_SIZE; ++i) {
            EXPECT_EQ(actual_hash[i], app_info.content.app_hash[i])
                << "Hash differ at index " << i;
        }
    }
}

TEST_F(SoftwareDownloadTest, VerifySwapAppHash) {
    {
        EXPECT_CALL(*mock_pico_interface_,
                    verify_hash(testing::_, ADDR_AS_U32(SWAP_APP_ADDRESS), 0))
            .WillOnce(testing::Return(false));

        EXPECT_FALSE(uut_.verify_swap_app_hash());
    }

    {
        unsigned char actual_hash[SHA256_DIGEST_SIZE]{'H', 'A', 'S', 'H'};
        auto copy_hash = [&actual_hash](
                             const unsigned char arg0[SHA256_DIGEST_SIZE],
                             const uint32_t arg1, const uint32_t arg2) {
            std::memcpy(actual_hash, arg0, SHA256_DIGEST_SIZE);
        };

        app_info_t app_info{};
        app_info.content.swap_app_size = __LINE__;
        std::memcpy(app_info.content.swap_app_hash, actual_hash,
                    SHA256_DIGEST_SIZE);
        std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);

        EXPECT_CALL(*mock_pico_interface_,
                    verify_hash(testing::_, ADDR_AS_U32(SWAP_APP_ADDRESS),
                                app_info.content.swap_app_size))
            .WillOnce(testing::DoAll(testing::Invoke(copy_hash),
                                     testing::Return(true)));

        EXPECT_TRUE(uut_.verify_swap_app_hash());

        for (int i = 0; i < SHA256_DIGEST_SIZE; ++i) {
            EXPECT_EQ(actual_hash[i], app_info.content.swap_app_hash[i])
                << "Hash differ at index " << i;
        }
    }
}

TEST_F(SoftwareDownloadTest, Reboot) {
    uint32_t delay{1};
    EXPECT_CALL(*mock_pico_interface_, reboot(delay));
    uut_.reboot(delay);

    delay = 0xFFFFFFFF;
    EXPECT_CALL(*mock_pico_interface_, reboot(8388));
    uut_.reboot(delay);
}

TEST_F(SoftwareDownloadTest, Restore) {
    uint32_t delay{1};
    EXPECT_FALSE(uut_.restore(delay));

    app_info_t app_info{};
    app_info.content.app_backed_up = TRUE_MAGIC_NUMBER;
    std::memcpy(g_app_info, app_info.raw, FLASH_PAGE_SIZE);
    {
        EXPECT_CALL(*mock_pico_interface_,
                    erase_flash(ADDR_AS_U32(APP_INFO_ADDRESS) - XIP_BASE,
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(false));
        EXPECT_FALSE(uut_.restore(delay));
    }

    {
        app_info_t actual_app_info{};
        auto copy_app_info = [&actual_app_info](uint32_t arg0,
                                                const uint8_t *arg1,
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
        EXPECT_CALL(*mock_pico_interface_, reboot(delay));
        EXPECT_TRUE(uut_.restore(delay));
        EXPECT_EQ(actual_app_info.content.app_restore_at_boot,
                  TRUE_MAGIC_NUMBER);
    }
}

}  // namespace PicoBootloader
