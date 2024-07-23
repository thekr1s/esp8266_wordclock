#!/bin/bash
export PATH=$PWD/esp-open-sdk/xtensa-lx106-elf/bin:$PATH
# Remove esptool. install system esp tool using:
# pip install esptool
rm -f esp-open-sdk/xtensa-lx106-elf/bin/esptool.py
rm -rf esp-open-sdk/esptool
