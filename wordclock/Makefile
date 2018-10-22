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

PROGRAM=ws2812_buffer
EXTRA_COMPONENTS = extras/i2s_dma extras/ws2812_i2s extras/sntp extras/dhcpserver extras/rboot-ota extras/mbedtls
#LIBS += esp-gdbstub
include ../esp-open-rtos/common.mk
