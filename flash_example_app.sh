#!/usr/bin/env bash

pushd build/example_app
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program PICO_BOOTLOADER_EXAMPLE_APP.elf verify reset exit"
popd
