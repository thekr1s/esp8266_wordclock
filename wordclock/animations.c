/*
 * animations.c
 *
 *  Created on: Mar 30, 2016
 *      Author: robert.wassens
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <task.h>

/* Including shared modules, which are used for whole project */

#include "clock_words.h"
#include "wordclock_main.h"

#include "AddressableLedStrip.h"
#include "font3x5.h"
#include "font4x5.h"
#include "font5x7.h"
#include "pacman.h"
#include "time.h"
#include "tetris_pieces.h"
#include "esp_glue.h"
#include "settings.h"
#include "controller.h"
#include "tetris.h"


#define MAX_MESSAGE_SIZE 60
static char g_message[MAX_MESSAGE_SIZE]="";


static const int8_t _PacmanPositions[][2] = { 
		{-5, -4},
		{-5, -3},
		{-5, -2},
		{-5, -1},
		{-5, 1},
		{-5, 2},
		{-5, 3},
		{-5, 3},
		{-5, 3},
		{-5, 3},
		{-5, 3},
		{-5, 3},
		{-5, 3},
		{-5, 3},
		{-5, 3},
		{-5, 3},
		{-4, 3},
		{-3, 4},
		{-2, 5},
		{-1, 6},
		{0, 7},
		{1, 8},
		{2, 9},
		{3, 10},
		{3, 10},
		{3, 10},
		{3, 10},
		{3, 10},
		{3, 10},
		{3, 10},
		{3, 10},
		{3, 10},
		{3, 10},
		{4, 10},
		{5, 10},
		{6, 10},
		{7, 10},
		{8, 10},
		{9, 10},
		{10, 10},
		{-100, -100}
};

const uint8_t _smiley[] = {
		0b00111100,
		0b01000010,
		0b10100101,
		0b10000001,
		0b10100101,
		0b10011001,
		0b01000010,
		0b00111100
};

const uint8_t _smiley2[] = {
		0b00111100,
		0b01000010,
		0b10101001,
		0b10101001,
		0b10000101,
		0b10111001,
		0b01000010,
		0b00111100
};


void ShowSplash(){
	if (g_settings.hardwareType == HARDWARE_13_13) {
		DisplayWord("By RMW");
	} else {
		AlsFill(5,5,5);
		AlsRefresh(ALSEFFECT_SHIFTDOWN);
		SleepNI(500);
		CWSet("by rmw", 5,5,80);
		AlsRefresh(ALSEFFECT_SHIFTRIGHT);
		SleepNI(5000);
		AlsFill(0,0,0);
		AlsRefresh(ALSEFFECT_FADE);
	}
}
void AnimationRandomFill(void)
{
	uint32_t h;
	AlsFill(0, 0, 0);
	for (h = 0; h < 50; h++) {
		if (Interrupted()) return;
		AlsSetRandom(g_brightness);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(100);
	}
	AlsFill(0, 0, 0);
	AlsRefresh(ALSEFFECT_ZIP);
	
}
static void ShowWord(char* word, uint8_t r, uint8_t g, uint8_t b, uint32_t delay) {
	AlsFill(0, 0, 0);
	CWSet(word, r, g, b);
	AlsRefresh(ALSEFFECT_NONE);
	Sleep(delay);
	
}
void TestAllWords(void)
{
	uint8_t r,g,b;
	r = ApplyBrightness(g_settings.color.r);
	g = ApplyBrightness(g_settings.color.g);
	b = ApplyBrightness(g_settings.color.b);

	if (g_settings.perfectImperfections == 1) { //for now use this setting but we need to find a better solution.

		AlsFill(0, 0, 0);
		CWSet("he", r, g, b);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(1000);
		CWSet("lise", r, g, b);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(500);
		CWSet("en", r, g, b);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(500);
		CWSet("evi", r, g, b);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(1000);
		CWSet("wacht", r, g, b);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(500);
		CWSet("even", r, g, b);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(2000);
		ShowTime(2000);
	} else {
		AlsFill(0, 0, 0);
		CWSet("het", r, g, b);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(500);
		CWSet("is", r, g, b);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(1000);

		ShowWord("een", r, g, b, 150);
		ShowWord("twee", r, g, b, 150);
		ShowWord("drie", r, g, b, 150);
		ShowWord("vier", r, g, b, 150);
		ShowWord("vijf", r, g, b, 150);
		ShowWord("zes", r, g, b, 150);
		ShowWord("zeven", r, g, b, 150);
		ShowWord("acht", r, g, b, 150);
		ShowWord("negen", r, g, b, 150);
		ShowWord("tien", r, g, b, 150);
		ShowWord("elf", r, g, b, 150);
		ShowWord("twaalf", r, g, b, 150);

		ShowTime(2000);

		AlsFill(0, 0, 0);
		CWSet("vijf_min", r, g, b);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(300);
		CWSet("tien_min", r, g, b);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(300);
		CWSet("voor", r, g, b);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(300);
		CWSet("over", r, g, b);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(700);

		AlsFill(0, 0, 0);
		CWSet("kwart", r, g, b);
		CWSet("half", r, g, b);
		CWSet("uur", r, g, b);
		AlsRefresh(ALSEFFECT_FADE);
		Sleep(700);

		AlsFill(0, 0, 0);
		CWSet("bijna", r, g, b);
		CWSet("nu", r, g, b);
		CWSet("geweest", r, g, b);
		AlsRefresh(ALSEFFECT_ZIP);
		Sleep(700);
	}
}

void DoPacman(void)
{
	uint32_t posIdx = 0;
	while (_PacmanPositions[posIdx][0] != -100) {
		if (Interrupted()) return;
		AlsFill(0, 0, 0);
		PCPutGhost(2, _PacmanPositions[posIdx][1], 0, g_brightness,
				g_brightness);
		PCPutPC(2, _PacmanPositions[posIdx][0], g_brightness,
				g_brightness, 0);
		posIdx++;
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(100);
	}
}

void DoCharacters()
{
	uint8_t r,g,b;
	uint32_t col = 17;
	uint32_t loop = 0;
	TColorIdx colorIdx = rand() % COLOR_COUNT;
	TColor c;

	char ch = (rand() % (130 - 32) ) + 32;
	
	while (loop != 3) {
		AlsFill(0,0,0);
		c = GetColorFromIdx(colorIdx);
		r = ApplyBrightness(c.r);
		g = ApplyBrightness(c.g);
		b = ApplyBrightness(c.b);

		F5x7WriteChar(2, col - 5, ch, r, g, b);
		col--;
		if (col == 0) {
			col = 17;
			ch = (rand() % (130 - 32) ) + 32;
			loop++;
			colorIdx = rand() % COLOR_COUNT;
		}

		AlsRefresh(ALSEFFECT_NONE);
		Sleep(100);
	}
}

void DisplaySmiley() {
	AlsFill(0, 0, 0);
	FontPutCharTD(8, 8, 1, 1, _smiley, g_brightness, g_brightness, 0);
	AlsRefresh(ALSEFFECT_RANDOM_EFFECT);
	Sleep(2000);	
	
}

void DisplaySmiley2() {
	AlsFill(0, 0, 0);
	FontPutCharTD(8, 8, 1, 1, _smiley2, g_brightness, g_brightness, 0);
	AlsRefresh(ALSEFFECT_RANDOM_EFFECT);
	Sleep(2000);

}

void Fire(uint32_t ms) {
	const uint8_t HEIGHT = 11;
	const uint8_t WIDTH = 11;
	uint8_t scn[ 11][ 11] = { { 0, }, };
	uint8_t r, c, t;
	uint32_t start = xTaskGetTickCount();


	AlsFill(0,0,0);
	while (GetTicksDiffMs(start, xTaskGetTickCount()) < ms) {
		if (Interrupted()) return;
		//advance fire
		for (r = 0; r < HEIGHT - 1; r++)
			for (c = 0; c < WIDTH; c++) {

				t = scn[r + 1][c] << 1;
				t += scn[r][c] >> 1;
				t += (c ? scn[r + 1][c - 1] : scn[r + 1][WIDTH - 1]) >> 1;
				t += (c == WIDTH - 1 ? scn[r + 1][0] : scn[r + 1][c + 1]) >> 1;
				t >>= 2;
				scn[r][c] = t;
				AlsSetLed(HEIGHT - 1 - r, c, t * 16, t * 6, 0);
			}

		//generate random seeds on the bottom
		for (c = 0; c < WIDTH; c++) {

			t = (rand() > 0xB0) ?
					rand() & 0x0F :
					(scn[HEIGHT - 1][c] ? scn[HEIGHT - 1][c] - 1 : 0);

			scn[HEIGHT - 1][c] = t;
			AlsSetLed(0, c, t * 16, t * 6, 0);
		}
		Sleep(110);
		AlsRefresh(ALSEFFECT_NONE);
	}
	AlsFill(0,0,0);
	AlsRefresh(ALSEFFECT_SHIFTDOWN);

}


void DisplayChecker(void) {
	uint8_t col, row;
	uint8_t val = 0;
	const uint8_t brightness = 10;
	
	
	while (TRUE) {
		val = val ? 0 : 1;
		AlsFill(0,0,0);		
		for (row = 0; row < _displaySize[0]; row++) {
			for (col = 0; col < _displaySize[1]; col++) {
				if ((row + col) % 2 == val) {
				   AlsSetLed(row, col, brightness, brightness, brightness);					
				}
			}
			
		}
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(1000);

	}
}

void DisplayWord(char* str) {
	int8_t pos;
	int8_t offset = 6;
	char c1 = ' ', c2 = ' ', c3 = ' ';
	int i;
	uint8_t r = ApplyBrightness(g_settings.color.r);
	uint8_t g = ApplyBrightness(g_settings.color.g);
	uint8_t b = ApplyBrightness(g_settings.color.b);

	for (i = 0; i < strlen(str) + 2; i++) {
		if (Interrupted()) return;
		c1 = c2;
		c2 = c3;
		if (i >= strlen(str)) {
			c3 = ' ';
		} else {
			c3 = str[i];
		}
		pos = 0;

		for (pos = 0; pos > -6; pos--) {
			AlsFill(BGRGB_FROM_SETTING);
			F5x7WriteChar(2, pos, c1, r, g, b);
			F5x7WriteChar(2, pos + offset, c2, r, g, b);
			F5x7WriteChar(2, pos + 2 * offset, c3, r, g, b);
			AlsRefresh(ALSEFFECT_NONE);
			Sleep(70);
		}

	}
}

void DisplayGreeting(){
	uint32_t h, m, s;
	
	TimeGet(&h, &m, &s);
	if (strlen(g_message) > 0) {
		DisplayWord(g_message);
	} else if (h < 4 || h >= 23) {
		DisplayWord("Good night");
	} else if (h < 12) {
		DisplayWord("Good morning");
	} else if (h < 18) {
		DisplayWord("Good afternoon");
	} else {
		DisplayWord("Good evening");
	} 
}

static const uint8_t _explode[][5] =
	{ 
		{ 
			0b0000000, 
			0b0000000, 
			0b0001000, 
			0b0000000, 
			0b0000000, 
		}, 
		{ 
			0b0000000, 
			0b0001000, 
			0b0011100, 
			0b0000000, 
			0b0000000, 
		}, 
		{ 
			0b0001000, 
			0b0110110, 
			0b0000000, 
			0b0000000, 
			0b0000000, 
		}, 
		{ 
			0b0001000, 
			0b0101010, 
			0b0000000, 
			0b0000000, 
			0b0000000, 
		}, 
		{ 
			0b0100010, 
			0b0001000, 
			0b0001000, 
			0b0000000, 
			0b0000000, 
		}, 
		{ 
			0b0000000, 
			0b1000001, 
			0b1001010, 
			0b0000000, 
			0b0000000, 
		}, 
		{ 
			0b0000000, 
			0b0000000, 
			0b1000010, 
			0b0101001, 
			0b0000000, 
		}, 
		{ 
			0b0000000, 
			0b0000000, 
			0b0000000, 
			0b0100001, 
			0b1001001, 
		}, 
		{ 
			0b0000000, 
			0b0000000, 
			0b0000000, 
			0b0000000, 
			0b1000001, 
		}, 
		{ 
			0b0000000, 
			0b0000000, 
			0b0000000, 
			0b0000000, 
			0b0000000, 
		}, 
    };

void fireworksExplode(int32_t row, int32_t col) {
	int i;
	uint8_t colorIdx = rand() % COLOR_COUNT;
	TColor c = GetColorFromIdx(colorIdx);
	uint8_t r = (c.r);
	uint8_t g = (c.g);
	uint8_t b = (c.b);
	row -= 3;
	col -= 3;
	
	for (i = 0; i < 10; i++) {
		AlsFill(0,0,0);
		FontPutCharTD(5,7,row,col,_explode[i], r, g, b);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(200);
		if (i > 5) {
			r /= 2;
			g /= 2;
			b /= 2;
		}
	}
}
void fireworks() {
	uint8_t start = rand() % 11;
	uint8_t dir = rand() % 2; // 0 = straight, 1 is diagonal
	uint8_t row = 0;
	uint8_t col = start;
	uint8_t colorIdx = rand() % COLOR_COUNT;
	TColor c = GetColorFromIdx(colorIdx);
	uint8_t r = ApplyBrightness(c.r);
	uint8_t g = ApplyBrightness(c.g);
	uint8_t b = ApplyBrightness(c.b);
	int i;
	
	for (i = 0; i < 7; i++) {
		if (Interrupted()) return;
		row++;
		if (dir == 1) {
			if (row % 2 == 0) {
				col += start < 5 ? 1 : -1;
			}
		}
		
		AlsFill(0,0,0);
		AlsSetLed(row, col, r, g, b);
		AlsRefresh(ALSEFFECT_NONE);
		Sleep(200);
	}
	fireworksExplode(row + 2, col);
}

void AnimationSetMessageText(char* txt){
	bzero(g_message, sizeof(g_message));
	strncpy(g_message, txt, sizeof(g_message) - 1);
}

char* AnimationGetMessageText(){
	return g_message;
}

void DoAnimation() {
	static uint32_t counter = 6;

	switch (counter) {
	case 0:
		DisplayGreeting();
		break;
	case 1:
		fireworks();
		//DoCharacters();
		break;
	case 2:
		DoTetris();
		break;
	case 3:
		#if 1
		TestAllWords();
		#endif
		break;
	case 4:
		DoPacman();
		break;
	case 5:
		AnimationRandomFill();
		break;
//	case 4:
//		DisplaySmiley();
//		break;
//	case 5:
//		Fire(5000);
//		break;
//	case 7:
//		DisplaySmiley2();
//		break;
//	case 8:
//		AnimationRandomFill();
//		break;
	default:
		// Start over
		counter = -1;
		break;
	}
	counter++;
}
