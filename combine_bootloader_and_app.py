import hashlib

add_download_app = True

# Constants
BOOTLOADER_SIZE = 0x8000
APP_INFO_SIZE = 0x1000
APP_STORAGE_SIZE = 0x1000
APP_INFO_FLAG_SIZE = 0x4
APP_SIZE = 0xFB000
BOOTLOADER_FILE = "build/bootloader/PICO_BOOTLOADER.bin"
APP_FILE = "build/example_app/PICO_BOOTLOADER_EXAMPLE_APP.bin"
APP_DOWNLOAD_FILE = "build/example_app/PICO_BOOTLOADER_EXAMPLE_APP_2.bin"
COMBINED_FILE = "build/PICO_BOOTLOADER_COMBINED.bin"

TRUE_VAL = 14253
FALSE_VAL = 0


def create_app_info(raw_file, download_raw_file=bytes(0)):
    app_hash = hashlib.sha256(raw_file).digest()
    print(
        f"Add {len(app_hash)} bytes app hash to binary:"
        f" {hashlib.sha256(raw_file).hexdigest()}"
    )
    if download_raw_file != bytes(0):
        download_app_hash = hashlib.sha256(download_raw_file).digest()
        print(
            f"Add {len(download_app_hash)} bytes download app hash to binary:"
            f" {hashlib.sha256(download_raw_file).hexdigest()}"
        )
    else:
        download_app_hash = bytes(32)
        print(
            f"Add {len(download_app_hash)} bytes download app hash to binary"
            " with all zeros"
        )

    app_size = len(raw_file).to_bytes(APP_INFO_FLAG_SIZE, "little")
    print(f"Add {len(app_size)} bytes app size")
    download_app_size = len(download_raw_file).to_bytes(
        APP_INFO_FLAG_SIZE, "little"
    )
    print(f"Add {len(download_app_size)} bytes download app size")
    if download_raw_file != bytes(0):
        app_download_flag = TRUE_VAL.to_bytes(APP_INFO_FLAG_SIZE, "little")
    else:
        app_download_flag = FALSE_VAL.to_bytes(APP_INFO_FLAG_SIZE, "little")

    app_restore_flag = FALSE_VAL.to_bytes(APP_INFO_FLAG_SIZE, "little")

    app_info = (
        app_hash
        + download_app_hash
        + app_size
        + download_app_size
        + app_download_flag
        + app_restore_flag
    )
    app_info_padding = bytes(
        [0xFF for _ in range(APP_INFO_SIZE - len(app_info))]
    )

    print(f"Add {len(app_download_flag)} bytes app downloaded flag")
    print(f"Add {len(app_restore_flag)} bytes app backed up flag")
    print(f"Add {len(app_info_padding)} bytes app info padding")

    return app_info + app_info_padding


with open(BOOTLOADER_FILE, "rb") as file:
    bootloader_raw_file = file.read()

bootloader_padding = bytes(
    [0xFF for _ in range(BOOTLOADER_SIZE - len(bootloader_raw_file))]
)

print(f"Add {len(bootloader_raw_file)} bytes bootloader")
print(f"Add {len(bootloader_padding)} bytes bootloader padding")

with open(APP_FILE, "rb") as file:
    app_raw_file = file.read()

with open(APP_DOWNLOAD_FILE, "rb") as file:
    app_download_raw_file = file.read()

with open(COMBINED_FILE, "wb") as file:

    storage_padding = bytes([0xFF for _ in range(APP_STORAGE_SIZE)])
    if add_download_app:
        app_padding = bytes(
            [0xFF for _ in range(APP_SIZE - len(app_raw_file))]
        )
        raw_content = (
            bootloader_raw_file
            + bootloader_padding
            + create_app_info(app_raw_file, app_download_raw_file)
            + storage_padding
            + app_raw_file
            + app_padding
            + app_download_raw_file
        )
        print(f"Add {len(storage_padding)} bytes storage padding")
        print(
            f"Add {len(app_raw_file)} bytes app with {len(app_padding)} bytes"
            " padding"
        )
        print(f"Add {len(app_download_raw_file)} bytes app")
    else:
        raw_content = (
            bootloader_raw_file
            + bootloader_padding
            + create_app_info(app_raw_file)
            + storage_padding
            + app_raw_file
        )
        print(f"Add {len(storage_padding)} bytes storage padding")
        print(f"Add {len(app_raw_file)} bytes app")

    file.write(raw_content)

print(f"Total {len(raw_content)} bytes written")
