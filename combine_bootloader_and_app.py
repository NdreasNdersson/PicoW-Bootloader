import hashlib

# import cfiles

BOOTLOADER_FILE = "build/bootloader/PICO_BOOTLOADER.bin"
APP_FILE = "build/example_app/PICO_BOOTLOADER_EXAMPLE_APP.bin"
COMBINED_FILE = "build/PICO_BOOTLOADER_COMBINED.bin"

with open(BOOTLOADER_FILE, "rb") as file:
    bootloader_raw_file = file.read()

with open(APP_FILE, "rb") as file:
    app_raw_file = file.read()

print(f"Add app hash to binary: {hashlib.sha256(app_raw_file).hexdigest()}")
app_hash = hashlib.sha256(app_raw_file).digest()
app_hash_padding = bytes([0xFF for _ in range(0x100 - len(app_hash))])
with open(COMBINED_FILE, "wb") as file:
    file.write(bootloader_raw_file + app_hash + app_hash_padding + app_raw_file)
