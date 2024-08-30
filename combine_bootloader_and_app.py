import hashlib

BOOTLOADER_SIZE = 0x8000
BOOTLOADER_FILE = "build/bootloader/PICO_BOOTLOADER.bin"
APP_FILE = "build/example_app/PICO_BOOTLOADER_EXAMPLE_APP.bin"
COMBINED_FILE = "build/PICO_BOOTLOADER_COMBINED.bin"

with open(BOOTLOADER_FILE, "rb") as file:
    bootloader_raw_file = file.read()

bootloader_padding = bytes(
    [0xFF for _ in range(BOOTLOADER_SIZE - len(bootloader_raw_file))]
)

print(f"Add {len(bootloader_raw_file)} bytes bootloader")
print(f"Add {len(bootloader_padding)} bytes bootloader padding")

with open(APP_FILE, "rb") as file:
    app_raw_file = file.read()

app_hash = hashlib.sha256(app_raw_file).digest()
app_size = len(app_raw_file).to_bytes(4, "little")
app_info = app_hash + app_size
app_info_padding = bytes([0xFF for _ in range(0x100 - len(app_info))])

print(
    f"Add {len(app_hash)} bytes app hash to binary: {hashlib.sha256(app_raw_file).hexdigest()}"
)
print(f"Add {len(app_size)} bytes app size")
print(f"Add {len(app_info_padding)} bytes app info padding")
print(f"Add {len(app_raw_file)} bytes app")

with open(COMBINED_FILE, "wb") as file:
    raw_content = (
        bootloader_raw_file
        + bootloader_padding
        + app_info
        + app_info_padding
        + app_raw_file
    )
    file.write(raw_content)

print(f"Total {len(raw_content)} bytes written")
