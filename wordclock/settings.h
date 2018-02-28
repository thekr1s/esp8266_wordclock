/*
 * settings.c
 *
 *  Created on: Oct 7, 2016
 *      Author: rutger.huijgen
 */
#ifndef SETTINGS_H_
#define SETTINGS_H_
#define TRUE true
#define FALSE false

#include "stdint.h"
#include "stdbool.h"

#include "animations.h"
#include "displaySettings.h"

#define FLASH_MAGIC 0xBABEBAB2
#define FLASH_EMPTY 0XFFFFFFFF
#define FLASH_INVALIDATED 0xB0B0BABE

typedef enum {
    HARDWARE_11_11 = 0,
    HARDWARE_13_13 = 1,
    HARDWARE_9_8 = 2,
} EHardwareType;

typedef struct {
    uint32_t magic;
    EAnimationType animation;
    EHardwareType hardwareType;
    const uint8_t aBrightness[BRIGHTNESS_COUNT];
    int8_t reserved0;
    int8_t brightnessOffset;
    int8_t colorIdx;
    int8_t bgColorIdx;
    uint32_t timerPeriodTicks;
    uint32_t isSummerTime;
    TColor aColors[COLOR_COUNT];
} TSettings;

typedef enum {
    USER_GUEST,
    USER_ROBERT_WASSENS,
    USER_RUTGER_HUIJGEN,
} EOwnerClock;

EOwnerClock ownerOfClock;

TSettings g_settings __attribute__((aligned(4)));



#define COLOR_FROM_IDX(i) g_settings.aColors[i]
#define RGB_FROM_COLOR_IDX(i)  ApplyBrightness(COLOR_FROM_IDX(i).r), \
    ApplyBrightness(COLOR_FROM_IDX(i).g),ApplyBrightness(COLOR_FROM_IDX(i).b)

#define COLOR_FROM_SETTING g_settings.aColors[g_settings.colorIdx]
#define RGB_FROM_SETTING  ApplyBrightness(COLOR_FROM_SETTING.r), \
    ApplyBrightness(COLOR_FROM_SETTING.g),ApplyBrightness(COLOR_FROM_SETTING.b)

#define BGCOLOR_FROM_SETTING g_settings.aColors[g_settings.bgColorIdx]
#define BGRGB_FROM_SETTING  ApplyBgBrightness(BGCOLOR_FROM_SETTING.r), \
    ApplyBgBrightness(BGCOLOR_FROM_SETTING.g),ApplyBgBrightness(BGCOLOR_FROM_SETTING.b)


void SettingsInit();
void SettingsRead();
void SettingsWrite();
void SettingsScheduleStore();
void SettingsCheckStore();
void SettingsClockReset();


#endif //SETTINGS_H_