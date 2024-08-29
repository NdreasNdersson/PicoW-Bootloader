#! /bin/bash

if [ ! -d build ]; then
  mkdir build;
fi

pushd build
cmake .. -G Ninja
ninja
popd

python3 pad_bootloader.py