#!/usr/bin/env bash

pushd example_app/build
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program PICO_BOOTLOADER_EXAMPLE_APP.elf verify reset exit"
popd
