#! /bin/bash

cmake -B build -G Ninja
cmake --build build

python3 pad_bootloader.py
python3 combine_bootloader_and_app.py
