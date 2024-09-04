#include "bootloader_lib.h"

#include <cstring>

#include "linker_definitions.h"

SoftwareDownload::SoftwareDownload() { read_app_info(); }

void SoftwareDownload::set_hash(
    const unsigned char app_hash[SHA256_DIGEST_SIZE]) {
    std::memcpy(m_app_info.content.swap_app_hash, app_hash, SHA256_DIGEST_SIZE);
}

void SoftwareDownload::set_size(const uint32_t &size) {
    m_app_info.content.swap_app_size = size;
}

auto SoftwareDownload::write_app(
    const unsigned char binary_block[FLASH_PAGE_SIZE]) -> bool {
    return true;
}

void SoftwareDownload::download_complete() {
    m_app_info.content.app_downloaded = true;
    m_app_info.content.app_backed_up = false;
    // Write
}

void SoftwareDownload::read_app_info() {
    std::memcmp(m_app_info.raw,
                reinterpret_cast<void *>(ADDR_AS_U32(__APP_INFO_ADDRESS)),
                FLASH_PAGE_SIZE);
}
