/*
 * settings.c
 *
 *  Created on: Oct 7, 2016
 *      Author: rutger.huijgen
 */
#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "stdint.h"
#include "stdbool.h"
#include "c_types.h"

#include "animations.h"
#include "displaySettings.h"
#include "AddressableLedStrip.h"

#define FLASH_MAGIC 0xBABEBAB4
#define FLASH_EMPTY 0XFFFFFFFF
#define FLASH_INVALIDATED 0xB0B0BABE

typedef enum {
    OTA_FW_RELEASE = 0,
    OTA_FW_DEBUG = 1,
} EOtaFwType;

typedef enum {
    HARDWARE_11_11 = 0,
    HARDWARE_13_13,
    HARDWARE_9_8,
    HARDWARE_13_13_V2,
    NR_OF_HARDWARE_TYPES,
} EHardwareType;

typedef enum {
    PIXEL_TYPE_RGB = 0,
    PIXEL_TYPE_RGBW,
    PIXEL_TYPE_RGBNW,
    PIXEL_TYPE_RGBWW,
    PIXEL_TYPE_RGB_N_W,
    NR_OF_PIXEL_TYPES,
} EPixelType;

typedef struct {
    uint32_t magic;
    EAnimationType animation;
    ETextEffect textEffect;
    EHardwareType hardwareType;
    EPixelType pixelType;
    uint32_t perfectImperfections;
    char hierbenikUrl[MAX_URL_SIZE];
    char hierbenikPort[MAX_PORT_SIZE];
    char hierbenikRequest[MAX_URL_SIZE];
    float hierbenikHomeLat;
    float hierbenikHomeLon;
    char otaFwUrl[MAX_URL_SIZE];
    char otaFwPort[MAX_PORT_SIZE];
    EOtaFwType otaFwType;
    const uint8_t aBrightness[BRIGHTNESS_COUNT];
    int8_t reserved0;
    int8_t brightnessOffset;
    TColor color;
    TColor bgColor;
    uint32_t timerPeriodTicks;
    uint32_t magic_end;
} TSettings;

TSettings g_settings __attribute__((aligned(4)));

#define COLOR_FROM_SETTING g_settings.color
#define RGB_FROM_SETTING  ApplyBrightness(COLOR_FROM_SETTING.r), \
    ApplyBrightness(COLOR_FROM_SETTING.g),ApplyBrightness(COLOR_FROM_SETTING.b)

#define BGCOLOR_FROM_SETTING g_settings.bgColor
#define BGRGB_FROM_SETTING  ApplyBgBrightness(BGCOLOR_FROM_SETTING.r), \
    ApplyBgBrightness(BGCOLOR_FROM_SETTING.g),ApplyBgBrightness(BGCOLOR_FROM_SETTING.b)


void SettingsInit();
void SettingsRead();
void SettingsWrite();
void SettingsScheduleStore();
void SettingsCheckStore();
void SettingsClockReset();

void SettingRandomColors();


#endif //SETTINGS_H_
