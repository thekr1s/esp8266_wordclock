# Get started on ubuntu 18.04 upto 20.04 (64 bits)
```
git clone https://github.com/thekr1s/esp8266_wordclock.git
cd esp8266_wordclock/esp-open-rtos
git submodule update --init --recursive
cd ..
tar -xvf esp-open-sdk.tgz 
sudo apt-get install make gcc g++
sudo pip install esptool
. env.sh
cd wordclock
make
```

# Program ESP
## Windows
- Install ESP8266Flasher : https://github.com/nodemcu/nodemcu-flasher
- Install the driver for the USB2Sereial http://www.wch.cn/download/CH341SER_ZIP.html

Start the ESP8266Flasher.exe en vul het onderstaande in:

- esp8266_wordclock\esp-open-rtos\bootloader\firmware_prebuilt\rboot.bin            @ 0x00000
- esp8266_wordclock\esp-open-rtos\bootloader\firmware_prebuilt\blank_config.bin     @ 0x01000
- esp8266_wordclock\wordclock\firmware\ws2812_buffer.bin                            @ 0x02000

## Linux
After building the firmware with 'make' as discribed above, type the folliwing command:
```
make do_flash
```

# Setup Wordclock
- hierbenik: the request is build by: sprintf(buf, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection:keep-alive\r\nAccept: */*\r\n\r\n", request, url);
- hierbenik request: "/get_with_age.php
