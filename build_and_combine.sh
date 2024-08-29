#! /bin/bash

if [ ! -d build ]; then
  mkdir build;
fi

pushd bootloader
if [ ! -d build ]; then
  mkdir build;
fi
cmake -B build -G Ninja
cmake --build build
popd

pushd example_app
if [ ! -d build ]; then
  mkdir build;
fi
cmake -B build -G Ninja
cmake --build build
popd

cp bootloader/build/*.bin build/
cp example_app/build/*.bin build/

python3 pad_bootloader.py
python3 combine_bootloader_and_app.py

