#include <gmock/gmock.h>

#include <cstdint>

#include "pico_interface.h"

class MockPicoInterface : public PicoInterface {
   public:
    MOCK_METHOD(void, watchdog_enable, (uint32_t delay_ms, bool paus_on_debug),
                (override));
    MOCK_METHOD(bool, store_to_flash,
                (uint32_t flash_offs, const uint8_t *data, size_t count),
                (override));
    MOCK_METHOD(bool, erase_flash, (uint32_t flash_offs, size_t count),
                (override));
    MOCK_METHOD(bool, verify_hash,
                (const unsigned char stored_sha256[SHA256_DIGEST_SIZE],
                 const uint32_t app_address, const uint32_t app_size_address),
                (override));
};