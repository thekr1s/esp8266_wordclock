# esp8266_wordclock


get started on ubuntu 14.04 / 16.04 server (64 bits)
===========================================
git clone https://github.com/thekr1s/esp8266_wordclock.git

cd esp8266_wordclock/esp-open-rtos

git apply ../esp-open-rtos.patch

cd ..

tar -xvf esp-open-sdk.tgz 

sudo apt-get install make python-serial eclipse gcc g++

. env.sh

cd wordclock

make

# Use eclipse
eclipse&

install C/C++ dev tools in eclipse

import "Existing projects into workspace" from source tree: esp-rtos and wordclock

build project 'wordclock'

program ESP
============
esp\esp-open-rtos\bootloader\firmware_prebuilt\rboot.bin            @ 0x00000

esp\esp-open-rtos\bootloader\firmware_prebuilt\blank_config.bin     @ 0x01000

esp\wordclock\firmware\ws2812_buffer.bin                            @ 0x02000
