#include "bootloader.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_bootloader_lib.h"

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
    MockBootloaderLib mock_bootloader_lib{};
    Bootloader uut{mock_bootloader_lib};

    EXPECT_CALL(mock_bootloader_lib, check_download_app_flag())
        .WillOnce(testing::Return(true));
    EXPECT_EQ(uut.check_download_app_flag(), true);

    EXPECT_CALL(mock_bootloader_lib, check_download_app_flag())
        .WillOnce(testing::Return(false));
    EXPECT_EQ(uut.check_download_app_flag(), false);
}

TEST_F(BootloaderTest, CheckRestoreAtBoot) {
    MockBootloaderLib mock_bootloader_lib{};
    Bootloader uut{mock_bootloader_lib};

    EXPECT_CALL(mock_bootloader_lib, check_restore_at_boot())
        .WillOnce(testing::Return(true));
    EXPECT_EQ(uut.check_restore_at_boot(), true);

    EXPECT_CALL(mock_bootloader_lib, check_restore_at_boot())
        .WillOnce(testing::Return(false));
    EXPECT_EQ(uut.check_restore_at_boot(), false);
}

TEST_F(BootloaderTest, SwapAppImages) {
    MockBootloaderLib mock_bootloader_lib{};
    Bootloader uut{mock_bootloader_lib};

    EXPECT_CALL(mock_bootloader_lib, swap_app_images());
    uut.swap_app_images();
}

TEST_F(BootloaderTest, VerifyAppHash) {
    MockBootloaderLib mock_bootloader_lib{};
    Bootloader uut{mock_bootloader_lib};

    EXPECT_CALL(mock_bootloader_lib, verify_app_hash())
        .WillOnce(testing::Return(true));
    EXPECT_EQ(uut.verify_app_hash(), true);

    EXPECT_CALL(mock_bootloader_lib, verify_app_hash())
        .WillOnce(testing::Return(false));
    EXPECT_EQ(uut.verify_app_hash(), false);
}

TEST_F(BootloaderTest, VerifySwapAppHash) {
    MockBootloaderLib mock_bootloader_lib{};
    Bootloader uut{mock_bootloader_lib};

    EXPECT_CALL(mock_bootloader_lib, verify_swap_app_hash())
        .WillOnce(testing::Return(true));
    EXPECT_EQ(uut.verify_swap_app_hash(), true);

    EXPECT_CALL(mock_bootloader_lib, verify_swap_app_hash())
        .WillOnce(testing::Return(false));
    EXPECT_EQ(uut.verify_swap_app_hash(), false);
}
