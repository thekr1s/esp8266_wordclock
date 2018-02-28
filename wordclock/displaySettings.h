/*
 * displaySettings.c
 *
 *  Created on: Oct 7, 2016
 *      Author: rutger.huijgen
 */
#ifndef DISPLAY_SETTINGS_H_
#define DISPLAY_SETTINGS_H_

#include "stdint.h"
#include "stdbool.h"

#define TRUE true
#define FALSE false

#define WORDCLOCK_ROWS_MAX 13 //The max values are used to allocate memory.
#define WORDCLOCK_COLLS_MAX 13
#define MAX_BRIGHTNESS_OFFSET  5
#define BRIGHTNESS_COUNT  12
#define COLOR_COUNT  14
#define LEDSTRIP_GPIO_NR 5 //the hardware GPIO used for controlling the display

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} TColor;

typedef enum {
    COLOR_WHITE,
    COLOR_BIT_PURPLE,
    COLOR_PURPLE,
    COLOR_MAGENTHA,
    COLOR_PINK,
    COLOR_RED,
    COLOR_ORANGE,
    COLOR_YELLOW,
    COLOR_YELLOW_GREEN,
    COLOR_GREEN,
    COLOR_BLUE_GREEN,
    COLOR_CYAN,
    COLOR_LIGHT_BLUE,
    COLOR_BLUE,
    COLOR_OFF
} TColorIdx;

uint8_t _displaySize[2]; //the cols and rows are set on init
uint8_t g_brightness;

bool DisplayInNightMode(void);
bool SetBrightness(void);
uint8_t ApplyBrightness(uint8_t color);
uint8_t ApplyBgBrightness(uint8_t color);
void wordClockDisplay_init(void);

#endif //DISPLAY_SETTINGS_H_