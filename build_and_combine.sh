#! /bin/bash
set -x

cmake -B build -G Ninja
cmake --build build

python3 combine_bootloader_and_app.py
