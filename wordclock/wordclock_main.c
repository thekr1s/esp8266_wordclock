/*
 * wordclock_main.c
 *
 *  Created on: Oct 26, 2016
 *      Author: robert
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <portmacro.h>
#include "espressif/esp_common.h"
#include "ws2812_i2s/ws2812_i2s.h"

#include <sntp.h>

#include <AddressableLedStrip.h>
#include <clock_words.h>
#include <font.h>

#include <wordclock_main.h>
#include <buttons.h>
#include <ldr.h>
#include <hier_ben_ik.h>
#include <sntp_client.h>
#include <wificfg.h>

#include "esp_glue.h"
#include <ota_basic.h>
#include <buttons.h>
#include "displaySettings.h"
#include "settings.h"
#include "controller.h"
#include "breakout.h"
#include "tetris.h"


static void ShowSome(uint32_t delayMS)
{
	uint8_t r,g,b;
	r = ApplyBrightness(g_settings.aColors[g_settings.colorIdx].r);
	g = ApplyBrightness(g_settings.aColors[g_settings.colorIdx].g);
	b = ApplyBrightness(g_settings.aColors[g_settings.colorIdx].b);

	AlsFill(0, 0, 0);
	switch (rand() % 3) {
	case 0:
		CWSet("wacht", r, g, b);
		break;
	case 1:
		CWSet("wacht", r, g, b);
		CWSet("even", r, g, b);
		break;
	case 2:
		CWSet("wacht", r, g, b);
		break;
	default:
		CWSet("wacht", r, g, b);
		CWSet("even", r, g, b);
		break;
	}

	AlsRefresh(ALSEFFECT_SHIFTRIGHT);
	Sleep(delayMS);

}

static void DisplayTimeSyncStatus()
{
	if (!sntp_client_time_valid()) {
		printf("timesync too old: red\n");
		AlsSetLed(_displaySize[0] - 1, 0, g_brightness, 0, 0);
	}
}

void DisplayTimeZone() {
	AlsFill(0,0,0);
	if (g_settings.isSummerTime) {
		F5x7WriteChar(1,0,'Z', g_brightness, g_brightness, g_brightness);
		F5x7WriteChar(1,6,'T', g_brightness, g_brightness, g_brightness);
	} else {
		F5x7WriteChar(1,0,'W', g_brightness, g_brightness, g_brightness);
		F5x7WriteChar(1,6,'T', g_brightness, g_brightness, g_brightness);
		
	}
	AlsRefresh(ALSEFFECT_NONE);
}

static void ShowDist(int dist)
{
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	const uint32_t farDist = 200000;
	const uint32_t closeDist = 110000;
	uint32_t t1, t2;

	if (dist > farDist) {
		r = 255;
	} else if (dist >= closeDist) {
		t1 = farDist- closeDist;
		t2 = dist - closeDist;
		g = 255 - (t2 * 255) / t1;
		r = 255 - g;
	} else {
		TColor c = g_settings.aColors[g_settings.colorIdx];
		if (c.g > (c.r + c.b)) {
			b = 255;
		} else {
			g = 255;
		}
		//r = (255 * dist) / closeDist;
	}

	r = ApplyBrightness(r);
	g = ApplyBrightness(g);
	b = ApplyBrightness(b);
	// Calculate meters
	t1 = dist % 1000;
	// Bottom row shows one pos per 100m
	t1 /= 100;
	AlsSetLed(t1, 0, r, g, b);

	// Calculate km
	dist /= 1000;
	t1 = dist / 10;
	t1 = t1 > 9 ? 9 : t1;
	t2 = dist % 10;
	AlsSetLed(t1, 10 - t2, r, g, b);
}

void TimeGet(uint32_t* h, uint32_t* m, uint32_t* s){
	time_t ts = time(NULL);
	if (g_settings.isSummerTime) {
		ts += 3600;
	}
	struct tm *pTM = localtime(&ts);
	*h = pTM->tm_hour;
    *m = pTM->tm_min;
    *s = pTM->tm_sec;
}

/**
 *
 * @param delayMS
 */
void ShowTime(int delayMS) {
	uint32_t h, m, s;
	static uint32_t prevMin = 0;
	uint32_t dist, age;
	TAlsEffects effect;
	uint32_t endTicks;
	bool DoReDisplay = true;

	endTicks = xTaskGetTickCount() + (delayMS / portTICK_PERIOD_MS);
	while (xTaskGetTickCount() < endTicks) {
		TimeGet(&h, &m, &s);
		if (ButtonHandleButtons()) {
			SetInterrupted(false);
			DoReDisplay = true;
			prevMin = m; // Prevent transition effect
			SettingsScheduleStore();
			endTicks = xTaskGetTickCount() + 5000/portTICK_PERIOD_MS;
		}
		if (Interrupted()) {
			// Interrupted via WEB interface
			SetInterrupted(false);
			SettingsScheduleStore();
			return;
		}

		DoReDisplay |= SetBrightness();
		if ((m != prevMin) && (g_settings.animation != ANIMATION_NONE) && !DisplayInNightMode()) {
			effect = ALSEFFECT_RANDOM_EFFECT;
			prevMin = m;
			DoReDisplay = true;
		} else {
			effect = ALSEFFECT_NONE;
		}

		if (DoReDisplay) {
			AlsFill(BGRGB_FROM_SETTING);
			if (g_settings.hardwareType == HARDWARE_13_13) {
				CWDisplayAccurateTime(h, m, s, RGB_FROM_SETTING);
			} else {
				CWDisplayTime(h, m, RGB_FROM_SETTING);
			}
			DisplayTimeSyncStatus();

			if (g_settings.colorIdx == COLOR_INDEX_RAINBOW) {
				AlsApplyFilter(ALSFILTER_RAINBOW);
			} 

//			ShowLdr();
			HbiGetDistAndAge(&dist, &age);
			if (age < 120 ) {
				ShowDist(dist);
			}
			
			if (g_settings.perfectImperfections == 1) {
				if (rand() % 50 == 0) {
					AlsSetRandom(g_brightness);
				}
			}
			AlsRefresh(effect);
		}
		DoReDisplay = false; 
		Sleep(300);
	}
}

static void ShowIpAddress(void){
	char str[20];
	struct ip_info info;
    if (sdk_wifi_get_ip_info(STATION_IF, &info)) {
		snprintf(str, sizeof(str), IPSTR, IP2STR(&info.ip));
		DisplayWord(str);
	}
}

void WordclockMain(void* p)
{
    (void) p;
	uint32_t timeShowDuration = 5000;

	SettingsInit();
	wordClockDisplay_init();

	if (g_settings.hardwareType == HARDWARE_13_13) {
		DisplayWord("By RMW");
	} else {
		ShowSplash();
	}

	// Wait for time set
	int count = 10;
	while ((time(NULL) < 10000) && (count-- > 0)) {
	    CWSet("wacht", 128,128,128);
	    CWSet("even", 128,128,128);
	    AlsRefresh(ALSEFFECT_NONE);
		SleepNI(1000);
		printf("time: %u\n",(uint32_t)time(NULL));
	}

	ShowIpAddress();
	
	while (time(NULL) < 10000) {
		SleepNI(5*1000);
	}

	while (1) {
		printf("woordklok task\n\n");
		switch (ControllerGameGet())
		{
			case GAME_BREAKOUT: DoBreakout(); break;
			case GAME_TETRIS: DoTetris(); break;
			default: break;
		}

		if (OtaIsBusy()) {
			DisplayWord("Updating firmware");
		} else if (g_settings.animation == ANIMATION_ALL_ON) {
			uint8_t orgBrightness = g_brightness;
			g_brightness = 10 + g_settings.brightnessOffset * 10;
			AlsFill(RGB_FROM_SETTING);
			AlsRefresh(ALSEFFECT_NONE);
			Sleep(5000);
			g_brightness = orgBrightness;
		} else if (g_settings.animation == ANIMATION_FIRE) {
			Fire(100000);
		} else if (g_settings.animation == ANIMATION_MESSAGE) {
			ShowTime(timeShowDuration * 3);
			DisplayGreeting();
		} else {
			ShowTime(timeShowDuration);
			// Once in a while shome something different
			if (g_settings.perfectImperfections == 1) {
				if (rand() % 100 == 0) {
					ShowSome(5000);
				}
			}
			timeShowDuration = 2000;
			if (g_settings.animation == ANIMATION_ALL && !DisplayInNightMode()) {
				DoAnimation();
			} else {
				timeShowDuration = 5000;
			}
		}
		if (ButtonsAnyPressed()) {
			// ShowTime handles the buttons...
			ShowTime(5000);
		}

		SetInterrupted(false);
		SettingsCheckStore();
		SleepNI(1000);
	}
}
