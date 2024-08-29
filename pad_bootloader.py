BOOTLOADER_SIZE = 0x8000
BOOTLOADER_FILE = "build/bootloader/PICO_BOOTLOADER.bin"

with open(BOOTLOADER_FILE, "rb") as file:
    raw_file = file.read()

bytes_to_pad = BOOTLOADER_SIZE - len(raw_file)
if bytes_to_pad < 0:
    raise ValueError(f"Bootloader '.bin' file can't be greater than {BOOTLOADER_SIZE}, but it's size is: {len(raw_file)}")

padding = bytes([0xFF for _ in range(bytes_to_pad)])
with open(BOOTLOADER_FILE, 'wb') as file:
    file.write(raw_file + padding)
