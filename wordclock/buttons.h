/*
 * wordclock_main.c
 *
 *  Created on: Oct 7, 2016
 *      Author: robbert.wassens
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

typedef enum {
    BUTTONMODE_COLOR,
    BUTTONMODE_BRIGHTNESS,
    BUTTONMODE_RED,
    BUTTONMODE_GREEN,
    BUTTONMODE_BLUE,
    BUTTONMODE_MINUTES,
    BUTTONMODE_HOURS,
    BUTTONMODE_DST, // Daylight Saving Time
    BUTTONMODE_ANIMATION,
    BUTTONMODE_LAST,
} ButtonMode;

void ButtonsInit(void);
bool ButtonsAllPressed();
bool ButtonsAnyPressed();
bool ButtonsUpPressed();
bool ButtonsModePressed();
bool ButtonsDownPressed();

void ButtonShowMode(ButtonMode mode);
void ButtonHandleUpDown(ButtonMode mode, bool up);
bool ButtonHandleButtons(void);

#endif //BUTTONS_H_
