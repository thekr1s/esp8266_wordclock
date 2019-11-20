get started on ubuntu 14.04 upto 19.10 (64 bits)
===========================================
git clone https://github.com/thekr1s/esp8266_wordclock.git

cd esp8266_wordclock/esp-open-rtos

git submodule update --init --recursive

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
Install ESP8266Flasher : https://github.com/nodemcu/nodemcu-flasher
Install the driver for the USB2Sereial http://www.wch.cn/download/CH341SER_ZIP.html

Start the ESP8266Flasher.exe en vul het onderstaande in:

esp8266_wordclock\esp-open-rtos\bootloader\firmware_prebuilt\rboot.bin            @ 0x00000
esp8266_wordclock\esp-open-rtos\bootloader\firmware_prebuilt\blank_config.bin     @ 0x01000
esp8266_wordclock\wordclock\firmware\ws2812_buffer.bin                            @ 0x02000
