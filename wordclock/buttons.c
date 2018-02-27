#include <stdint.h>
#include <stdbool.h>
#include "espressif/esp_common.h"
#include <esp/gpio.h>
#include <time.h>
#include <sntp_client.h>

#include <event_handler.h>
#include "buttons.h"
#include "AddressableLedStrip.h"
#include "font.h"
#include "displaySettings.h"
#include "wordclock_main.h"
#include "settings.h"
#include "esp_glue.h"


static const uint8_t _buttondownGpio = 4;   // NodeMCU pin D2
static const uint8_t _buttonModeGpio = 14;  //             D5
static const uint8_t _buttonUpGpio = 12;    //             D6

void gpio04_interrupt_handler(void){
	EvtHdlButtonStateChange();
}

void gpio14_interrupt_handler(void){
	EvtHdlButtonStateChange();
}

void gpio12_interrupt_handler(void){
	EvtHdlButtonStateChange();
}

void ButtonsInit(void){
    gpio_enable(_buttondownGpio, GPIO_INPUT);
    gpio_set_pullup(_buttondownGpio, true, true);
    gpio_set_interrupt(_buttondownGpio, GPIO_INTTYPE_EDGE_ANY);
    gpio_enable(_buttonModeGpio, GPIO_INPUT);
    gpio_set_pullup(_buttonModeGpio, true, true);
    gpio_set_interrupt(_buttonModeGpio, GPIO_INTTYPE_EDGE_ANY);
    gpio_enable(_buttonUpGpio, GPIO_INPUT);
    gpio_set_pullup(_buttonUpGpio, true, true);
    gpio_set_interrupt(_buttonUpGpio, GPIO_INTTYPE_EDGE_ANY);

}

bool ButtonsDownPressed() {
	return (gpio_read(_buttondownGpio) == 0);
}
bool ButtonsModePressed() {
	return (gpio_read(_buttonModeGpio) == 0);
}
bool ButtonsUpPressed() {
	return (gpio_read(_buttonUpGpio) == 0);
}

bool ButtonsAllPressed() {

	return ButtonsDownPressed() && ButtonsModePressed() && ButtonsUpPressed();
}

bool ButtonsAnyPressed() {

	return ButtonsDownPressed() || ButtonsModePressed() || ButtonsUpPressed();
}

void ButtonShowMode(ButtonMode mode)
{
    AlsFill(0,0,0);
    switch (mode) {
    case BUTTONMODE_MINUTES:
        F5x7WriteChar(1,0,'M', g_brightness, g_brightness, g_brightness);
        F5x7WriteChar(1,5,'i', g_brightness, g_brightness, g_brightness);
        break;

    case BUTTONMODE_HOURS:
        F5x7WriteChar(1,0,'U', g_brightness, g_brightness, g_brightness);
        F5x7WriteChar(1,6,'U', g_brightness, g_brightness, g_brightness);
        break;

    case BUTTONMODE_DST:
        DisplayTimeZone();
        break;

//  case BUTTONMODE_BRIGHTNESS:
//      F4x5WriteChar(1,0,'B', g_brightness, g_brightness, g_brightness);
//      F4x5WriteChar(1,5,'r', g_brightness, g_brightness, g_brightness);
//      break;

    case BUTTONMODE_RED:
        F5x7WriteChar(1,0,'R', g_brightness, g_brightness, g_brightness);
        F5x7WriteChar(1,6,'O', g_brightness, g_brightness, g_brightness);
        break;

    case BUTTONMODE_GREEN:
        F5x7WriteChar(1,0,'G', g_brightness, g_brightness, g_brightness);
        F5x7WriteChar(1,6,'R', g_brightness, g_brightness, g_brightness);
        break;

    case BUTTONMODE_BLUE:
        F5x7WriteChar(1,0,'B', g_brightness, g_brightness, g_brightness);
        F5x7WriteChar(1,6,'L', g_brightness, g_brightness, g_brightness);
        break;

    case BUTTONMODE_COLOR:
        F5x7WriteChar(1,0,'K', g_brightness, g_brightness, g_brightness);
        F5x7WriteChar(1,6,'L', g_brightness, g_brightness, g_brightness);
        break;

    case BUTTONMODE_ANIMATION:
        F5x7WriteChar(1,0,'A', g_brightness, g_brightness, g_brightness);
        F5x7WriteChar(1,6,'N', g_brightness, g_brightness, g_brightness);
        break;

    case BUTTONMODE_BRIGHTNESS:
        F5x7WriteChar(1,0,'B', g_brightness, g_brightness, g_brightness);
        F5x7WriteChar(1,6,'R', g_brightness, g_brightness, g_brightness);
        break;

    default:
        F5x7WriteChar(1,0,'E', g_brightness, g_brightness, g_brightness);
        F5x7WriteChar(1,6,'R', g_brightness, g_brightness, g_brightness);
        break;
    }
    AlsRefresh(ALSEFFECT_NONE);
}

/**
 *
 * @param mode
 * @param up  true: up false: down
 */
void ButtonHandleUpDown(ButtonMode mode, bool up)
{
//  uint32_t h,m,s;


    switch (mode) {
//  case BUTTONMODE_MINUTES:
//      TimeGet(&h, &m, &s);
//      m = up ? m + 1 : m - 1;
//      if (m > 0xf0000000) {
//          // wrap the minute
//          m = 59;
//      }
//      //m -= m % 5;
//      m %= 60;
//      TimeSet(h, m, 0, false);
//      break;
//
//  case BUTTONMODE_HOURS:
//      TimeGet(&h, &m, &s);
//      h = up ? h + 1 : h - 1;
//      if (h > 0xf0000000) {
//          // wrap the hour
//          h = 23;
//      }
//      h %= 24;
//      TimeSet(h, m, s, false);
//      break;

    case BUTTONMODE_DST:
        g_settings.isSummerTime = ! g_settings.isSummerTime;
        DisplayTimeZone();
        SleepNI(1000);
        break;

//  case BUTTONMODE_BRIGHTNESS:
//      _brightnessIdx = up ? _brightnessIdx + 1 : _brightnessIdx - 1;
//      if (_brightnessIdx == BRIGHTNESS_COUNT) {
//          _brightnessIdx = BRIGHTNESS_COUNT - 1;
//      }
//      if (_brightnessIdx == -1) {
//          _brightnessIdx = 0;
//      }
//
//
//      g_brightness = _aBrightness[_brightnessIdx];
//
//      break;

    case BUTTONMODE_COLOR:
        g_settings.colorIdx = up ? g_settings.colorIdx + 1: g_settings.colorIdx - 1;
        if (g_settings.colorIdx == COLOR_COUNT) {
            g_settings.colorIdx = COLOR_COUNT - 1;
        }
        if (g_settings.colorIdx == -1) {
            g_settings.colorIdx = 0;
        }
        AlsFill(0,0,0);
        F5x7WriteChar(1,0,'K', RGB_FROM_SETTING);
        F5x7WriteChar(1,6,'L', RGB_FROM_SETTING);
        AlsRefresh(ALSEFFECT_NONE);
        SleepNI(1000);
        break;

    case BUTTONMODE_BRIGHTNESS:
        g_settings.brightnessOffset = up ? g_settings.brightnessOffset + 1: g_settings.brightnessOffset - 1;
        if (g_settings.brightnessOffset > MAX_BRIGHTNESS_OFFSET) {
            g_settings.brightnessOffset = MAX_BRIGHTNESS_OFFSET;
        }
        if (g_settings.brightnessOffset == -1) {
            g_settings.brightnessOffset = 0;
        }
        AlsFill(0,0,0);
        F5x7WriteChar(1,4,0x30 + g_settings.brightnessOffset, g_brightness, g_brightness, g_brightness);
        AlsRefresh(ALSEFFECT_NONE);
        SleepNI(1000);
        break;

    case BUTTONMODE_RED:
        g_settings.aColors[g_settings.colorIdx].r = up ? g_settings.aColors[g_settings.colorIdx].r + 10 : g_settings.aColors[g_settings.colorIdx].r - 10;
        break;

    case BUTTONMODE_GREEN:
        g_settings.aColors[g_settings.colorIdx].g = up ? g_settings.aColors[g_settings.colorIdx].g + 10 : g_settings.aColors[g_settings.colorIdx].g - 10;
        break;

    case BUTTONMODE_BLUE:
        g_settings.aColors[g_settings.colorIdx].b = up ? g_settings.aColors[g_settings.colorIdx].b + 10 : g_settings.aColors[g_settings.colorIdx].b - 10;
        break;

    case BUTTONMODE_ANIMATION:
        g_settings.animation += up ? 1 : -1;
        if ((g_settings.animation) < 0) g_settings.animation = 0;
        if (g_settings.animation >= ANIMATION_FIRE) g_settings.animation = ANIMATION_FIRE - 1;
        AlsFill(0,0,0);
        if (g_settings.animation == ANIMATION_ALL) {
            F5x7WriteChar(1,0,'O', g_brightness, g_brightness, g_brightness);
            F5x7WriteChar(1,6,'N', g_brightness, g_brightness, g_brightness);
        } else  if (g_settings.animation == ANIMATION_TRANSITION) {
            F5x7WriteChar(1,0,'T', g_brightness, g_brightness, g_brightness);
            F5x7WriteChar(1,6,'R', g_brightness, g_brightness, g_brightness);
        } else {
            F5x7WriteChar(1,0,'O', g_brightness, g_brightness, g_brightness);
            F5x7WriteChar(1,6,'F', g_brightness, g_brightness, g_brightness);
        }
        AlsRefresh(ALSEFFECT_NONE);
        SleepNI(1000);
        break;

    default:
        break;
    }

}

/**
 * @returns: true when button was pressed and handled, else false
 */
bool ButtonHandleButtons(void) {
    bool buttonHandled = false;
    static ButtonMode mode = BUTTONMODE_COLOR;
    static uint32_t lastButtonPress = 0;

    if ((time(NULL) - lastButtonPress) > 60) {
        // Set default to color mode after some idle time
        mode = BUTTONMODE_COLOR;
    }
    if (ButtonsAllPressed()) {
        SettingsClockReset();
    }

    if (ButtonsModePressed()) {
        mode += 1;
        if (mode == BUTTONMODE_RED) {
            // Disable the RGB setting
            mode += 3;
        }
        if (sntp_client_time_valid() && mode == BUTTONMODE_MINUTES) {
            // Skip the minutes and hour setting as the clock is already set;
            mode += 2;
        }
        mode %= BUTTONMODE_LAST;
        ButtonShowMode(mode);
        SleepNI(1000);
        buttonHandled = true;
    }


    if (ButtonsUpPressed()) {
        ButtonHandleUpDown(mode, true);
        buttonHandled = true;
    }
    if (ButtonsDownPressed()) {
        ButtonHandleUpDown(mode, false);
        buttonHandled = true;
    }
    if (buttonHandled) {
        lastButtonPress = time(NULL);
    }
    return buttonHandled;
}
