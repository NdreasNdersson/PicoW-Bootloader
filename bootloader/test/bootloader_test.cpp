#include "bootloader.h"

#include <gtest/gtest.h>

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
    Bootloader uut;
    EXPECT_EQ(uut.check_download_app_flag(), true);
}

TEST_F(BootloaderTest, CheckRestoreAtBoot) {
    Bootloader uut;
    EXPECT_EQ(uut.check_restore_at_boot(), true);
}

TEST_F(BootloaderTest, SwapAppImages) {
    Bootloader uut;
    uut.swap_app_images();
}
