/*
 * settings.c
 *
 *  Created on: Oct 7, 2016
 *      Author: rutger.huijgen
 */

#include <stdio.h> // printf
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sysparam.h>
#include "espressif/esp_common.h"

#include <esp_glue.h>
#include <settings.h>
#include <AddressableLedStrip.h>

#define SETTINGS_KEY "wc-cfg"
#define HW_SETTINGS_KEY "wc-hw-cfg"

static volatile uint32_t g_storeTS = 0;
static const TSettings g_settings_default __attribute__((aligned(4))) = {
    FLASH_MAGIC,
    ANIMATION_TRANSITION,
    TEXTEFFECT_NONE, // textEffect
#ifdef BUILD_BY_RUTGER
    0,   // perfectImperfections
#else
    1,   // perfectImperfections
#endif
    "",  // hierbenikUrl
    "80",   // hierbenikPort
    "/get_with_age.php",  // hierbenikRequest
    0.0, // home lat
    0.0, // home lon
#ifdef BUILD_BY_RUTGER
    "http://rutger798.mynetgear.com",  // otaFwUrl
    "8090",   // otaFwPort
#else
    "http://download.wssns.nl",  // otaFwUrl
    "80",   // otaFwPort
#endif
    OTA_FW_RELEASE, //otaFwType
    {2, 4, 7, 10, 15, 25, 40, 60, 90, 120, 150, 170},
    1, // reserved
    0, // brightnessOffset
    {255,255,255}, // colorIdx = White
    {0,0,0},       // bgColorIdx = Black
    52220, // timerPeriodTicks
    {0}, //reserved[]
};

static const THwSettings g_hw_settings_default __attribute__((aligned(4))) = {
    FLASH_MAGIC_HW,
#ifdef BUILD_BY_RUTGER
    HARDWARE_13_13_V2,
#else
    HARDWARE_11_11,
#endif
    PIXEL_TYPE_RGB,
    {0}, //reserved[]
};

void SettingsInit() {
    size_t actual_size;
    g_settings.magic = 0;
    g_hw_settings.magic = 0;
    sysparam_get_data_static(SETTINGS_KEY, (uint8_t*)&g_settings, sizeof(g_settings), &actual_size, NULL);
    if (actual_size == sizeof(g_settings) && (g_settings.magic == FLASH_MAGIC)) {
        printf("Valid settings read from sysparams flash\r\n");
    } else {
        printf("No valid settings found, size %d, magic: %08x\r\n", actual_size, g_settings.magic);
        printf("Use default settings\r\n");
        memcpy((uint8_t*)&g_settings, (uint8_t*)&g_settings_default, sizeof(TSettings));
    }

    sysparam_get_data_static(HW_SETTINGS_KEY, (uint8_t*)&g_hw_settings, sizeof(g_hw_settings), &actual_size, NULL);
    if (actual_size == sizeof(g_hw_settings) && (g_hw_settings.magic == FLASH_MAGIC_HW)) {
        printf("Valid hardware settings read from sysparams flash\r\n");
    } else {
        printf("No valid hardware settings found, size %d, magic: %08x\r\n", actual_size, g_hw_settings.magic);
        printf("Use default settings\r\n");
        memcpy((uint8_t*)&g_hw_settings, (uint8_t*)&g_hw_settings_default, sizeof(TSettings));
    }
}

void SettingsWrite(){
    printf("########## saved config to flash##########\r\n");

    AlsFill(0, ApplyBrightness(100), 0);
    AlsRefresh(ALSEFFECT_NONE);

    sysparam_set_string(SETTINGS_KEY, "");
    sysparam_set_data(SETTINGS_KEY, (uint8_t*)&g_settings, sizeof(g_settings), true);

    sysparam_set_string(HW_SETTINGS_KEY, "");
    sysparam_set_data(HW_SETTINGS_KEY, (uint8_t*)&g_hw_settings, sizeof(g_hw_settings), true);
    SleepNI(300);
}

void SettingsScheduleStore(){
    g_storeTS = time(NULL);
}

void SettingsCheckStore(){
    if (g_storeTS != 0) {
        uint32_t dt = time(NULL) - g_storeTS;
        if (dt > 60) {
            g_storeTS = 0;
            SettingsWrite();
        }
    }

}

void SettingsClockReset(){
    sysparam_set_string(SETTINGS_KEY, "");
    AlsFill(20,0,0);
    AlsRefresh(ALSEFFECT_NONE);
    SleepNI(2000);
    sdk_system_restart();
}

static TColor RndCol() {
    TColor c = {rand()%256,rand()%256,rand()%256};
    uint8_t m = MAX(c.r, c.g);
    m = MAX(c.b, m);
    float f = 1.0*255/m;
    printf("%d %f\n", m, f);
    c.r = c.r * f;
    c.g = c.g * f;
    c.b = c.b * f;
    
    return c;
}

void SettingRandomColors(){
    g_settings.color = RndCol();
    TColor bgc = RndCol();
    bgc.r /= 10;
    bgc.g /= 10;
    bgc.b /= 10;
    g_settings.bgColor = bgc;
}