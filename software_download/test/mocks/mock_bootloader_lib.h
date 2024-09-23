#include <gmock/gmock.h>

#include <cstdint>

#include "pico_interface.h"

class MockPicoInterface : public PicoInterface {
   public:
    MOCK_METHOD(bool, flash_safe_execute,
                (void (*func)(void *), void *param,
                 uint32_t enter_exit_timeout_ms),
                (override));
    MOCK_METHOD(void, flash_range_erase, (uint32_t flash_offs, size_t count),
                (override));
    MOCK_METHOD(void, flash_range_program,
                (uint32_t flash_offs, const uint8_t *data, size_t count),
                (override));
};
