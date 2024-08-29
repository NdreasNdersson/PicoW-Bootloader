#!/usr/bin/env bash

pushd build/
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program PICO_BOOTLOADER_COMBINED.bin verify reset exit 0x10000000"
popd
