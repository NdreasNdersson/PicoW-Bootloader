#include "bootloader.h"

#include "bootloader_lib.h"

Bootloader::Bootloader(BootloaderLib &bootloader_lib)
    : bootloader_lib_{bootloader_lib} {}

auto Bootloader::check_download_app_flag() -> bool {
    return bootloader_lib_.check_download_app_flag();
}

auto Bootloader::check_restore_at_boot() -> bool {
    return bootloader_lib_.check_restore_at_boot();
}

void Bootloader::swap_app_images() { bootloader_lib_.swap_app_images(); }

auto Bootloader::verify_app_hash() -> bool {
    return bootloader_lib_.verify_app_hash();
}

auto Bootloader::verify_swap_app_hash() -> bool {
    return bootloader_lib_.verify_swap_app_hash();
}
