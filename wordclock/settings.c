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

static volatile uint32_t g_storeTS = 0;
static const TSettings g_settings_default __attribute__((aligned(4))) = {
     FLASH_MAGIC,
     ANIMATION_TRANSITION,
     HARDWARE_11_11,
     1,   // perfectImperfections
     "",  // hierbenikUrl
     "80",   // hierbenikPort
     "/get_with_age.php",  // hierbenikRequest
     0.0, // home lat
     0.0, // home lon
     "rwassens.ddns.net",  // otaFwUrl
     "10070",   // otaFwPort
     {2, 4, 7, 10, 15, 25, 40, 60, 90, 120, 150, 170},
     1, // reserved
     0, // brightnessOffset
     0, // colorIdx
     0, // bgColorIdx
     52220, // timerPeriodTicks
     FALSE, // isSummerTime
     {
        {255,255,255},
        {185,150,255},
        {128,000,255},
        {255,000,255},
        {255,000,128},
        {255,000,000},
        {255,128,000},
        {255,255,000},
        {128,255,000},
        {000,255,000},
        {000,255,128},
        {000,255,255},
        {000,128,255},
        {000,000,255},
        {255,255,255},//dummy used for rainbow
     },
     FLASH_MAGIC
};

void SettingsInit() {
    size_t actual_size;
    g_settings.magic = 0;
    sysparam_get_data_static(SETTINGS_KEY, (uint8_t*)&g_settings, sizeof(g_settings), &actual_size, NULL);
    if (actual_size == sizeof(g_settings) && (g_settings.magic == FLASH_MAGIC)) {
        printf("Valid settings read from sysparams flash\r\n");
    } else {
        printf("No valid settings found, size %d, magic: %08x\r\n", actual_size, g_settings.magic);
        printf("Use default settings\r\n");
        memcpy((uint8_t*)&g_settings, (uint8_t*)&g_settings_default, sizeof(TSettings));
    }
}

void SettingsWrite(){
    printf("########## saved config to flash##########\n");

    AlsFill(0, ApplyBrightness(100), 0);
    AlsRefresh(ALSEFFECT_NONE);

    sysparam_set_string(SETTINGS_KEY, "");
    sysparam_set_data(SETTINGS_KEY, (uint8_t*)&g_settings, sizeof(g_settings), true);
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
