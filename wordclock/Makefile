# Makefile for the ws2812 buffer write


SVNVERSION="todo"
BUILD_DATE=$(shell date +"%Y%m%d-%H:%M")
$(info svn version $(SVNVERSION))
$(info build date $(BUILD_DATE))
#Force the compiler to rebuild the version.c
$(shell touch version.c)

CPPFLAGS += -DSVNVERSION=$(SVNVERSION) -DBUILD_DATE=$(BUILD_DATE)

PROGRAM=ws2812_buffer
EXTRA_COMPONENTS = extras/i2s_dma extras/ws2812_i2s extras/sntp extras/dhcpserver extras/rboot-ota extras/mbedtls
#LIBS += esp-gdbstub
include ../esp-open-rtos/common.mk
