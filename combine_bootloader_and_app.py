BOOTLOADER_FILE = "build/bootloader/PICO_BOOTLOADER.bin"
APP_FILE = "build/example_app/PICO_BOOTLOADER_EXAMPLE_APP.bin"
COMBINED_FILE = "build/PICO_BOOTLOADER_COMBINED.bin"

with open(BOOTLOADER_FILE, "rb") as file:
    bootloader_raw_file = file.read()

with open(APP_FILE, "rb") as file:
    app_raw_file = file.read()

with open(COMBINED_FILE, 'wb') as file:
    file.write(bootloader_raw_file + app_raw_file)
