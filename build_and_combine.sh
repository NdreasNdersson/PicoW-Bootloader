#! /bin/bash
set -eux

cmake -B build -G Ninja
cmake --build build

python3 combine_bootloader_and_app.py
