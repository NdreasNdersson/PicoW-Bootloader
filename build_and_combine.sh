#! /bin/bash
set -eux

if [ $# -eq 0 ]; then
    debug=0
    cmake --preset ninja-release
    cmake --build --preset ninja-release
else
    if [ "${1^^}" = "DEBUG" ]; then
        debug=1
    else
        debug=0
    fi
fi

if [ $debug = 1 ]; then
    cmake --preset ninja-debug
    cmake --build --preset ninja-debug
    ./combine_bootloader_and_app.py \
    --bootloader-file "build/ninja-debug/bootloader/PICO_BOOTLOADER.bin" \
    --app-file "build/ninja-debug/example_app/PICO_BOOTLOADER_EXAMPLE_APP.bin"
else
    cmake --preset ninja-release
    cmake --build --preset ninja-release
    ./combine_bootloader_and_app.py \
    --bootloader-file "build/ninja-release/bootloader/PICO_BOOTLOADER.bin" \
    --app-file "build/ninja-release/example_app/PICO_BOOTLOADER_EXAMPLE_APP.bin"
fi
