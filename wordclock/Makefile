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

CPPFLAGS += -DVERSION=$(VERSION) -DBUILD_DATE=$(BUILD_DATE)
EXTRA_CFLAGS += -DSNTP_CHECK_RESPONSE=2

# enable debugging for the wifi config tasks
#EXTRA_CFLAGS += -DWIFI_CONFIG_DEBUG

# enable debugging for the sntp server
#EXTRA_CFLAGS += -DSNTP_LOGD -DSNTP_LOGD_WITH_PRINTF

# For the mDNS responder included with lwip:
#EXTRA_CFLAGS += -DLWIP_MDNS_RESPONDER=1 -DLWIP_NUM_NETIF_CLIENT_DATA=1 -DLWIP_NETIF_EXT_STATUS_CALLBACK=1

# Avoid writing the wifi state to flash when using wificfg.
#EXTRA_CFLAGS += -DWIFI_PARAM_SAVE=0

PROGRAM=ws2812_buffer
EXTRA_COMPONENTS = extras/wifi-config-v2 extras/http-parser extras/i2s_dma extras/ws2812_i2s extras/sntp extras/dhcpserver extras/rboot-ota extras/mbedtls
LIBS += m
include ../esp-open-rtos/common.mk

do_flash:
	../esp-open-sdk/esptool/esptool.py write_flash 0 ../esp-open-rtos/bootloader/firmware_prebuilt/rboot.bin
	../esp-open-sdk/esptool/esptool.py write_flash 0x01000 ../esp-open-rtos/bootloader/firmware_prebuilt/blank_config.bin
	# Suspect: Using high speed causes all leds turn on. Voltage drop. CPU stalls. FLASH defect. Strange behavior afterwards.
	# ../esp-open-sdk/esptool/esptool.py --baud 861600 write_flash 0x02000 firmware/ws2812_buffer.bin -ff 80m 
	# So use lower speed.
	../esp-open-sdk/esptool/esptool.py --baud 115200 write_flash 0x02000 firmware/ws2812_buffer.bin -ff 80m 
