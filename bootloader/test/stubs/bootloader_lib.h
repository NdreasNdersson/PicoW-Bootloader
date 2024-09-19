#include <cstdint>

constexpr int SHA256_DIGEST_SIZE{32};
constexpr int FLASH_PAGE_SIZE{32};

class SoftwareDownload {
   public:
    SoftwareDownload() = default;
    void init_download(const uint32_t &size) {}
    void set_hash(const unsigned char app_hash[SHA256_DIGEST_SIZE]) {}
    auto write_app(const unsigned char binary_block[FLASH_PAGE_SIZE]) -> bool {
        return true;
    }
    void download_complete() {}
    auto verify_app_hash() -> bool { return true; }
    auto verify_swap_app_hash() -> bool { return true; }
    void reboot(uint32_t delay) {}
    auto restore(uint32_t delay) -> bool { return true; }

   protected:
    auto check_download_app_flag() const -> bool { return true; }
    auto check_restore_at_boot() const -> bool { return true; }
    void swap_app_images() {}
};
