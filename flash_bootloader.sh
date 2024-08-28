#!/usr/bin/env bash

pushd bootloader/build/
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program PICO_BOOTLOADER.elf verify reset exit"
popd
