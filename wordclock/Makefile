# Makefile for the ws2812 buffer write


GITVERSION=$(shell git rev-parse --short HEAD)
# Set dirty suffix depending on output of "git status".
ifneq ($(shell git status --porcelain),)
    dirty=M
else
    dirty=C
endif
VERSION = $(GITVERSION)$(dirty)

BUILD_DATE=$(shell date +"%Y%m%d-%H:%M")
$(info version $(VERSION))
$(info build date $(BUILD_DATE))
#Force the compiler to rebuild the version.c
$(shell touch version.c)

# enable debugging for the sntp server
#EXTRA_CFLAGS += -DSNTP_LOGD -DSNTP_LOGD_WITH_PRINTF

CPPFLAGS += -DVERSION=$(VERSION) -DBUILD_DATE=$(BUILD_DATE)
EXTRA_CFLAGS += -DSNTP_CHECK_RESPONSE=2

PROGRAM=woordklok_dbg
EXTRA_COMPONENTS = extras/i2s_dma extras/ws2812_i2s extras/sntp extras/dhcpserver extras/rboot-ota extras/mbedtls  extras/http_client_ota
LIBS += m
include ../esp-open-rtos/common.mk

strip_crc:
	# strip CRC from the end of the file. The Wordclocks software rejects the file size.
	$(shell cp $(FW_FILE) $(FW_FILE)_crc_striped)
	$(shell truncate -s -4 $(FW_FILE)_crc_striped)
	echo stripped CRC from $(FW_FILE)_crc_striped

do_flash: strip_crc
	esptool.py write_flash 0 ../esp-open-rtos/bootloader/firmware_prebuilt/rboot.bin -ff 40m -fs 32m -fm dio
	esptool.py write_flash 0x01000 ../esp-open-rtos/bootloader/firmware_prebuilt/blank_config.bin -ff 40m -fs 32m -fm dio
	esptool.py --baud 115200 write_flash 0x02000 $(FW_FILE)_crc_striped -ff 40m -fs 32m -fm dio
