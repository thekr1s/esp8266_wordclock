#!/bin/bash
PROJECT_HOME=$(dirname '$0')
export PATH=$PROJECT_HOME/esp-open-sdk/xtensa-lx106-elf/bin:$PATH
export PATH=$PWD/esp-open-sdk/xtensa-lx106-elf/bin:$PATH
export SDK_PATH=$PROJECT_HOME/ESP8266_RTOS_SDK
export BIN_PATH=$PROJECT_HOME/bin
# Remove esptool. install system esp tool using:
# pip install esptool
rm -f esp-open-sdk/xtensa-lx106-elf/bin/esptool.py
rm -rf esp-open-sdk/esptool
