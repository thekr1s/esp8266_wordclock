# fw_githash_bbb705e.bin
Firmware build from git commit bbb705e. This is non reproducable as the used
version of esp_open_rtos is not available anymore.

First erase the complete flash:
esptool.py -p COM9 erase_flash
Flash it at address 0:
esptool.py -p COM9 --baud 230400 write_flash 0x00000 fw_githash_bbb705e.bin -ff 40m -fs 32m -fm dio
