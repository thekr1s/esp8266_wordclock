# fw_githash_bbb705e.bin
Firmware build from git commit bbb705e. This is non reproducable as the used
version of esp_open_rtos is not available anymore.
Flash it at address 0:
../esp-open-sdk/esptool/esptool.py --baud 230400 write_flash 0x00000 old_fw_full.bin -ff 40m -fs 32m -fm dio
