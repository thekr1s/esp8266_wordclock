/*
 * displaySettings.c
 *
 *  Created on: Oct 7, 2016
 *      Author: rutger.huijgen
 */

#include <stdio.h> // printf
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#include "FreeRTOS.h"
#include "task.h"

#include "espressif/esp_common.h"
#include <espressif/esp_wifi.h>
#include "esp/uart.h" // uart_set_baud
#include "ws2812_i2s/ws2812_i2s.h"
#include "displaySettings.h"
#include "esp_glue.h"
#include <ldr.h>
#include <AddressableLedStrip.h>
#include <clock_words.h>
#include <font.h>
#include "settings.h"

uint16_t _aLdrLevels[] = {808, 800, 700, 550, 300, 100, 50, 30, 10, 5};
static bool g_inNightMode = false;

TColor _definedColors[] = {
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
{000,000,255}};

TColor GetColorFromIdx(TColorIdx idx)
{
    return _definedColors[idx];
}

bool DisplayInNightMode(void)
{
    return g_inNightMode;
}

/**
 * @return TRUE if brighness changed, else false
 */
bool SetBrightness(void)
{
    uint32_t i = 0;
    static uint8_t br = 100;
    bool res = FALSE;
    static uint32_t lastMeasurementTicks = 0;

    if (GetTicksDiffMs(lastMeasurementTicks, xTaskGetTickCount()) > 1000) {
        uint16_t v;
        LdrGetValue16(&v);

        while ((i < BRIGHTNESS_COUNT) && (v < _aLdrLevels[i])) {
            i++;
        }
        // When the lowest level light is measured, dimm the leds to the max, else use brightness setting.
        if (i == 0){
            g_inNightMode = TRUE;
        } else {
            i+= g_settings.brightnessOffset;
            g_inNightMode = FALSE;

        }

        if (i >= BRIGHTNESS_COUNT) {
            i = BRIGHTNESS_COUNT - 1;
        }
        if (v >= _aLdrLevels[i]) {
            br = g_settings.aBrightness[i];
        } else {
            br = 220; // Maximum brightness
        }
        //printf("LDR: %d, br: %d %d\n", v, br, g_brightness);
        lastMeasurementTicks = xTaskGetTickCount();
    }

    if (br > g_brightness){
        g_brightness += 1;
        if (br - g_brightness > 10) {
            g_brightness += 3;
        }
        res = TRUE;
    } else if (br < g_brightness) {
        g_brightness -= 1;
        if (g_brightness - br > 10) {
            g_brightness -= 3;
        }
        res = TRUE;
    }
    return res;
}

uint8_t ApplyBrightness(uint8_t color)
{
    uint32_t t;
    t = color * g_brightness;
    return (uint8_t)(t / 255);

}
uint8_t ApplyBgBrightness(uint8_t color)
{
    // If backgrondcolor white, then turn led off.
    if (g_settings.bgColor.r == 0 &&
        g_settings.bgColor.g == 0 &&
        g_settings.bgColor.b == 0) return 0;
    if (g_settings.bgColor.r == 255 &&
        g_settings.bgColor.g == 255 &&
        g_settings.bgColor.b == 255) return 0;
    uint32_t t;
    t = color * g_brightness;
    return (uint8_t)(t / 255);
}

static void WS2812_I2S_WriteData(uint8_t* p, uint32_t length){  
    if (g_settings.pixelType == PIXEL_TYPE_RGB) {
        ws2812_i2s_update((ws2812_pixel_t*) p, PIXEL_RGB);
    } else {
        ws2812_i2s_update((ws2812_pixel_t*) p, PIXEL_RGBW);
    }
}

void wordClockDisplay_init(void)
{
    //Set the brightness on boot
    g_brightness = 70;

    // Configure the GPIO
    gpio_enable(LEDSTRIP_GPIO_NR, GPIO_OUTPUT);

    printf("###ws2812##hardwareTyp is %d\n", g_settings.hardwareType);
    if (g_settings.hardwareType == HARDWARE_13_13 || g_settings.hardwareType == HARDWARE_13_13_V2 || g_settings.hardwareType == HARDWARE_13_13_NOT_ACCURATE) {
        _displaySize[0] = 13;
        _displaySize[1] = 13;
    } else if (g_settings.hardwareType == HARDWARE_11_11) {
        _displaySize[0] = 11;
        _displaySize[1] = 11;
    } else if (g_settings.hardwareType == HARDWARE_9_8) {
        _displaySize[0] = 9;
        _displaySize[1] = 8;
    } else {
        _displaySize[0] = 11;
        _displaySize[1] = 11;
    }

    CWInit(_displaySize[0],_displaySize[1]);
    if (g_settings.pixelType == PIXEL_TYPE_RGB) {
        ws2812_i2s_init(_displaySize[0] *_displaySize[1], PIXEL_RGB);
    } else {
        ws2812_i2s_init(_displaySize[0] *_displaySize[1], PIXEL_RGBW);
    }
    AlsInit(_displaySize[0], _displaySize[1], WS2812_I2S_WriteData);

    FontInit(AlsSetLed);
}