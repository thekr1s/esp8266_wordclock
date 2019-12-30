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

#include "espressif/esp_common.h"

#include <esp_glue.h>
#include <settings.h>
#include <AddressableLedStrip.h>

#define SETTINGS_ADDRESS 0x7e000

static volatile uint32_t g_storeTS = 0;
TSettings g_settings_default __attribute__((aligned(4))) = {
     FLASH_MAGIC,
     ANIMATION_TRANSITION,
     HARDWARE_11_11,
     1,   // perfectImperfections
     "",  // hierbenikUrl
     "",   // hierbenikPort
     "",  // hierbenikRequest
     "",  // otaFwUrl
     "",   // otaFwPort
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
};

void SettingsInit() {
    uint32_t magic;

    sdk_spi_flash_read(SETTINGS_ADDRESS, (uint32_t*) &magic, sizeof(magic));
    if (magic == FLASH_MAGIC) {
        // Valid settings found
        sdk_spi_flash_read(SETTINGS_ADDRESS, (uint32_t*) &g_settings, sizeof(g_settings));
    } else {
        //if there is no config stored in flash use the default
        memcpy((uint8_t*)&g_settings, (uint8_t*)&g_settings_default, sizeof(TSettings));
    }
}

void SettingsRead() {
    uint32_t magic;

    sdk_spi_flash_read(SETTINGS_ADDRESS, (uint32_t*) &magic, sizeof(magic));
    if (magic == FLASH_MAGIC) {
        // Valid settings found
        AlsFill(0, ApplyBrightness(100), 0);
        AlsRefresh(ALSEFFECT_NONE);
        sdk_spi_flash_read(SETTINGS_ADDRESS, (uint32_t*) &g_settings, sizeof(g_settings));
    } else {
        //if there is no config stored in flash use the default
        memcpy((uint8_t*)&g_settings, (uint8_t*)&g_settings_default, sizeof(TSettings));
    }
}

void SettingsWrite(){
    printf("########## saved config to flash##########\n");

    AlsFill(0, ApplyBrightness(100), 0);
    AlsRefresh(ALSEFFECT_NONE);

    sdk_spi_flash_erase_sector(SETTINGS_ADDRESS / sdk_flashchip.sector_size);
    sdk_spi_flash_write(SETTINGS_ADDRESS, (uint32_t*)&g_settings, sizeof(g_settings));

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
    sdk_spi_flash_erase_sector(SETTINGS_ADDRESS / sdk_flashchip.sector_size);
    AlsFill(20,0,0);
    AlsRefresh(ALSEFFECT_NONE);
    SleepNI(2000);
    sdk_system_restart();
}
