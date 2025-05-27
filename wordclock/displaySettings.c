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
#include "rgb2.h"

#define CLAMP(x, lo, hi) (((x) < (lo)) ? (lo) : ((x) > (hi)) ? (hi) : (x))

TPixel _frameCopy[WORDCLOCK_ROWS_MAX * WORDCLOCK_COLLS_MAX];
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

TColor GetColorFromIdx(TColorIdx idx) {
    return _definedColors[idx];
}

bool DisplayInNightMode(void) {
    return g_inNightMode;
}

void applyBrightnessTransition(uint8_t target) {
    if (target > g_brightness) {
        g_brightness += 1;
        if (target - g_brightness > 10) {
            g_brightness += 3;
        }
    } else if (target < g_brightness) {
        g_brightness -= 1;
        if (g_brightness - target > 10) {
            g_brightness -= 3;
        }
    }
}

/**
 * @return TRUE if brighness changed, else false
 */
bool SetBrightness(void) {
    int8_t idx;
    uint16_t ldrValueAvr;
    static uint8_t targetBrightness = 255;
    static uint8_t mafIdx = 0;
    static uint16_t ldrValues[3] = {128, 128, 128};
    static uint32_t lastMeasurementTicks = 0;

    if (GetTicksDiffMs(lastMeasurementTicks, xTaskGetTickCount()) > 333) {
        ldrValues[mafIdx] = LdrGetValue16();
        mafIdx = (mafIdx + 1) % 3;
        ldrValueAvr = (ldrValues[0] + ldrValues[1] + ldrValues[2]) / 3;

        for (idx = 0; idx < BRIGHTNESS_LUT_SIZE; idx++) {
            if (ldrValueAvr < g_hw_settings.ldrThresholds[idx]) {
                break;
            }
        }

        // When the lowest level light is measured set night mode
        g_inNightMode = (idx == 0) ? TRUE : FALSE;
        idx+= g_settings.brightnessOffset;
        idx = CLAMP(idx, 0, BRIGHTNESS_LUT_SIZE-1);

        targetBrightness = g_hw_settings.brightnessLUT[idx];
        //printf("LDR: %d, idx: %d, br: %d/%d\n", ldrValueAvr, idx, targetBrightness, g_brightness);
        lastMeasurementTicks = xTaskGetTickCount();
    }

    if (targetBrightness != g_brightness) {
        applyBrightnessTransition(targetBrightness);
        return TRUE;
    } else {
        return FALSE; // No change
    }
}

uint8_t ApplyBrightness(uint8_t color) {
    uint32_t t;
    t = color * g_brightness;
    return (uint8_t)(t / 255);
}
uint8_t ApplyBgBrightness(uint8_t color) {
    // If backgrondcolor white, then turn led off.
    if (g_settings.bgColor.r == 0 &&
        g_settings.bgColor.g == 0 &&
        g_settings.bgColor.b == 0) return 0;
    if (g_settings.bgColor.r == 255 &&
        g_settings.bgColor.g == 255 &&
        g_settings.bgColor.b == 255) return 0;
    return ApplyBrightness(color);
}

static void WS2812_I2S_WriteData(TPixel* pixels, uint32_t nrOfPixels) {  
    if (g_hw_settings.pixelType == PIXEL_TYPE_RGB) {
        ws2812_i2s_update((ws2812_pixel_t*) pixels, PIXEL_RGB);
    } else {
        // For the RGBW leds the white LED is used to show common value,
        // The calculated RGBW value should not be stored in the frame, therefor make a copy
        for (int i = 0; i < nrOfPixels; i++) {
            rgb2rgbw(_frameCopy[i], pixels[i], g_hw_settings.pixelType);
        }
        ws2812_i2s_update((ws2812_pixel_t*) _frameCopy, PIXEL_RGBW);
    }
}

void wordClockDisplay_init(void)
{
    //Set the brightness on boot
    g_brightness = 70;

    // Configure the GPIO
    gpio_enable(LEDSTRIP_GPIO_NR, GPIO_OUTPUT);

    printf("###ws2812##hardwareTyp is %d\n", g_hw_settings.hardwareType);
    if (g_hw_settings.hardwareType == HARDWARE_13_13 || g_hw_settings.hardwareType == HARDWARE_13_13_V2 || g_hw_settings.hardwareType == HARDWARE_13_13_V2_1 || g_hw_settings.hardwareType == HARDWARE_13_13_NOT_ACCURATE) {
        _displaySize[0] = 13;
        _displaySize[1] = 13;
    } else if (g_hw_settings.hardwareType == HARDWARE_11_11) {
        _displaySize[0] = 11;
        _displaySize[1] = 11;
    } else if (g_hw_settings.hardwareType == HARDWARE_9_8) {
        _displaySize[0] = 9;
        _displaySize[1] = 8;
    } else {
        _displaySize[0] = 11;
        _displaySize[1] = 11;
    }

    CWInit();
    if (g_hw_settings.pixelType == PIXEL_TYPE_RGB) {
        ws2812_i2s_init(_displaySize[0] *_displaySize[1], PIXEL_RGB);
    } else {
        ws2812_i2s_init(_displaySize[0] *_displaySize[1], PIXEL_RGBW);
    }
    AlsInit(_displaySize[0], _displaySize[1], WS2812_I2S_WriteData);

    FontInit(AlsSetLed);
}