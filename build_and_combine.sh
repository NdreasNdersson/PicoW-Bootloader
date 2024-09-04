#! /bin/bash
set -eux

cmake -B build -G Ninja
cmake --build build

./combine_bootloader_and_app.py
