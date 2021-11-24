/*
 * LedStrip.cpp
 *
 *  Created on: May 9, 2015
 *      Author: robert.wassens
 */
#include <stdio.h> // printf
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "esp_glue.h"
#include "AddressableLedStrip.h"
#include "displaySettings.h"
#include "settings.h"
#include "rgb2.h"


#define ALS_MAX_LED_COUNT  (WORDCLOCK_ROWS_MAX * WORDCLOCK_COLLS_MAX)

#define FLAG_IS_BG 0x1

static TPixel _bgColor;

typedef struct TFrame {
	TPixel buff[ALS_MAX_LED_COUNT];
	uint8_t flags[ALS_MAX_LED_COUNT];
} TFrame;

// Array of two frames holding the pixels and the flags per pixel
static TFrame _frames[2];
#define CURRFAME_IDX 0
#define NEXTFAME_IDX 1

static uint32_t _rows = 0;
static uint32_t _cols = 0;
TAlsWriteFunction _writeFunction = NULL;

extern uint32_t TimerTickCount;
extern uint8_t g_brightness;

void AlsInit(uint32_t rows, uint32_t cols, TAlsWriteFunction writeFunction) 
{
	_writeFunction = writeFunction;
	_rows = rows;
	_cols = cols;
}

uint32_t AlsGetRows() {
	return _rows;
}

uint32_t AlsGetCols(){
	return _cols;
}

static void DisplayFrame(int frameIdx) {
	TPixel* frame = _frames[frameIdx].buff;
	uint8_t* flags = _frames[frameIdx].flags;

	
	for (int idx = 0; idx < _cols * _rows; idx++) {
		if (flags[idx] & FLAG_IS_BG) {
			frame[idx][RED_IDX] = _bgColor[RED_IDX];
			frame[idx][GREEN_IDX] = _bgColor[GREEN_IDX];
			frame[idx][BLUE_IDX] = _bgColor[BLUE_IDX];
		}
		if (g_settings.pixelType != PIXEL_TYPE_RGB) {
			rgb2rgbw(frame[idx], g_settings.pixelType);
		}
	}
	// int dbgidx = _cols * _rows - 1;
	// printf("After: %d ; %d ; %d ; %d\n", frame[dbgidx][RED_IDX], frame[dbgidx][GREEN_IDX], frame[dbgidx][BLUE_IDX], frame[dbgidx][WHITE_IDX]);
	
	_writeFunction((uint8_t*)frame, _cols * _rows * ALS_BYTES_PER_LED);

	if (frameIdx == NEXTFAME_IDX) {
		memcpy(_frames[CURRFAME_IDX].buff, _frames[NEXTFAME_IDX].buff, sizeof(_frames[0].buff));
		memcpy(_frames[CURRFAME_IDX].flags, _frames[NEXTFAME_IDX].flags, sizeof(_frames[0].flags));
	}
}

void AlsSetBackgroundColor(uint8_t red, uint8_t green, uint8_t blue) {
	_bgColor[RED_IDX] = red;
	_bgColor[GREEN_IDX] = green;
	_bgColor[BLUE_IDX] = blue;

}

void AlsFill(uint8_t red, uint8_t green, uint8_t blue){
	int idx;
	for (idx = 0; idx < _cols * _rows; idx++) {
	
		_frames[NEXTFAME_IDX].buff[idx][RED_IDX] = red;
		_frames[NEXTFAME_IDX].buff[idx][GREEN_IDX] = green;
		_frames[NEXTFAME_IDX].buff[idx][BLUE_IDX] = blue;
		if (red == 0 && green == 0 && blue == 0) {
			_frames[NEXTFAME_IDX].flags[idx] |= FLAG_IS_BG;
		} else {
			_frames[NEXTFAME_IDX].flags[idx] &= ~FLAG_IS_BG;
		}
	}	
}

static void SetLedInFrame(int frame_idx, uint32_t row, uint32_t col, uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t idx;
	TPixel* frame = _frames[frame_idx].buff;
	uint8_t* flags = _frames[frame_idx].flags;

	idx =  (_rows - 1 - row) * _cols;
	if (row >= _rows || col >= _cols) {
		//Ignore calls when index out of range
		return; 
	}
	if (row % 2 == 0) {
		idx += col;
	} else {
		idx += _cols - 1 - col;		
	}
	frame[idx][RED_IDX] = r;
	frame[idx][GREEN_IDX] = g;
	frame[idx][BLUE_IDX] = b;
	if (r + g + b <= _bgColor[RED_IDX] + _bgColor[GREEN_IDX] + _bgColor[BLUE_IDX]){
		// pixel is set to black. set it as background pixel
		flags[idx] |=  FLAG_IS_BG; 
	} else {
		flags[idx] &= ~ FLAG_IS_BG; // clear the background flag.
	}

}

void AlsSetLed(uint32_t row, uint32_t col, uint8_t red, uint8_t green, uint8_t blue)
{
	SetLedInFrame(NEXTFAME_IDX, row, col, red, green, blue);
}

static void GetLedFromFrame(int frame_idx, uint32_t row, uint32_t col, uint8_t* r, uint8_t* g, uint8_t* b)
{
	TPixel* frame = _frames[frame_idx].buff;
	uint8_t* flags = _frames[frame_idx].flags;

	if (row >= _rows || col >= _cols) {
		//Ignore calls when index out of range
		return; 
	}

	uint32_t idx =  (_rows - 1 - row) * _cols;

	if (row % 2 == 0) {
		idx += col;
	} else {
		idx += _cols - 1 - col;		
	}
	if (flags[idx] & FLAG_IS_BG) {
		*r = *g = *b = 0;
	} else {
		*r = frame[idx][RED_IDX];
		*g = frame[idx][GREEN_IDX];
		*b = frame[idx][BLUE_IDX];
	}	
}

void AlsGetLed(uint32_t row, uint32_t col, uint8_t* pRed, uint8_t* pGreen, uint8_t* pBlue)
{
	GetLedFromFrame(NEXTFAME_IDX, row, col, pRed, pGreen, pBlue);
}

static void ShiftHorizontal(bool shiftRight)
{
	uint8_t r,g,b;
	int32_t col, row;
	
	for (col = _cols - 1; col >= 0; col--) {
		for(row = 0; row < _rows; row++) {
			if (col >= 0){
				GetLedFromFrame(CURRFAME_IDX, row, col - 1, &r, &g, &b);
				SetLedInFrame(CURRFAME_IDX, row, col, r, g, b);
			} else {
				SetLedInFrame(CURRFAME_IDX, row, col, BGRGB_FROM_SETTING);
			}
		}
	}
	
}

static void ShiftVertical(bool shiftDown)
{
	uint8_t r,g,b;
	int32_t col, row;
	
	for (row = 0; row < _rows; row++) {
		for(col = 0; col < _cols; col++) {
			if (row < _rows - 1){
				GetLedFromFrame(CURRFAME_IDX, row + 1, col, &r, &g, &b);
				SetLedInFrame(CURRFAME_IDX, row, col, r, g, b);
			} else {
				SetLedInFrame(CURRFAME_IDX, row, col, BGRGB_FROM_SETTING);
			}
		}
	}
	
}

static void RefreshRight(bool displayNext, uint32_t delay)
{
	uint32_t i, row;
	uint8_t r, g, b;

	for (i = 0; i < _cols; i++) {
		ShiftHorizontal(TRUE);
		if (displayNext) {
			for (row = 0; row < _rows; row++){		
				GetLedFromFrame(NEXTFAME_IDX, row, _cols - i - 1, &r, &g, &b);
				SetLedInFrame(CURRFAME_IDX, row, 0, r, g, b);
				
			}
		}
		DisplayFrame(CURRFAME_IDX);
		Sleep(delay);
	}
	
}

static void RefreshUp(bool displayNext, uint32_t delay)
{
	uint32_t i, col;
	uint8_t r, g, b;

	for (i = 0; i < _cols; i++) {
		ShiftVertical(TRUE);
		if (displayNext) {
			for (col = 0; col < _cols; col++){		
				GetLedFromFrame(NEXTFAME_IDX, i, col, &r, &g, &b);
				SetLedInFrame(CURRFAME_IDX, _rows - 1, col, r, g, b);
				
			}
		}
		DisplayFrame(CURRFAME_IDX);
		Sleep(delay);
	}
	
}

uint8_t Zip(uint8_t a, uint32_t b) 
{
	if (a != 0 && b != 0) {
		b = ((uint32_t)a + b) / 2;
	} else if (a != 0) {
		b = a;
	}
	
	return b;	
}

static void FadeOut() {
	uint8_t row, col;
	uint8_t r, g, b;
	bool done = FALSE;;
	
	while (!done) {
		done = TRUE;
		for (row = 0; row < _rows; row++) {
			for (col = 0; col < _cols; col++) {
				GetLedFromFrame(CURRFAME_IDX, row, col, &r, &g, &b);
				r /= 2;
				g /= 2;
				b /= 2;
				if (r != 0 || g != 0 || b != 0) {
					done = FALSE;
				}
				SetLedInFrame(CURRFAME_IDX, row, col, r, g, b);
			}
			DisplayFrame(CURRFAME_IDX);
		}
		Sleep(100);
	}
}

static void RefreshZip(bool displayNext, uint32_t delay)
{
	uint32_t row, col, c, centre;
	uint8_t r, g, b;
	uint8_t r1, g1, b1;
	centre = _rows / 2;
	
	for (c = 0; c < (_rows / 2); c++) {
		for (col = 0; col < _cols; col++) {
			row = c;
			GetLedFromFrame(CURRFAME_IDX, row, col, &r, &g, &b);
			GetLedFromFrame(CURRFAME_IDX, row + 1, col, &r1, &g1, &b1);
			r = Zip(r, r1);
			g = Zip(g, g1);
			b = Zip(b, b1);
			SetLedInFrame(CURRFAME_IDX, row + 1, col, r, g, b);
			SetLedInFrame(CURRFAME_IDX, row, col, 0, 0, 0);
			row = _rows - c - 1;
			
			GetLedFromFrame(CURRFAME_IDX, row, col, &r, &g, &b);
			GetLedFromFrame(CURRFAME_IDX, row - 1, col, &r1, &g1, &b1);
			r = Zip(r, r1);
			g = Zip(g, g1);
			b = Zip(b, b1);
			SetLedInFrame(CURRFAME_IDX, row - 1, col, r, g, b);
			SetLedInFrame(CURRFAME_IDX, row, col, 0, 0, 0);
		}
		DisplayFrame(CURRFAME_IDX);
		Sleep(delay);
		if (delay > 120) {
			delay /= 2;
		}
	}
	for (col = 0; col < _cols; col++) {
		GetLedFromFrame(CURRFAME_IDX, centre, col, &r, &g, &b);
		r = r < 64 ? r * 4 : 255;
		g = g < 64 ? g * 4 : 255;
		b = b < 64 ? b * 4 : 255;
		SetLedInFrame(CURRFAME_IDX, centre, col, r, g, b);
	}
	DisplayFrame(CURRFAME_IDX);
	Sleep(100);

	for (c = 0; c < 10; c++) {
		for (col = 0; col < _cols; col++) {
			GetLedFromFrame(CURRFAME_IDX, centre, col, &r, &g, &b);
			r /= 2;
			g /= 2;
			b /= 2;
			SetLedInFrame(CURRFAME_IDX, centre, col, r, g, b);
		}
		DisplayFrame(CURRFAME_IDX);
		Sleep(100);	    	
	}
	if(displayNext) {
		DisplayFrame(NEXTFAME_IDX);
		
	}
}

static void RefreshBlend(void)
{
	
}

static void RefreshFade(void)
{
	FadeOut();
	DisplayFrame(NEXTFAME_IDX);

}

static void RefreshWipe(void)
{
	uint8_t wipedR[_rows];
	uint8_t wipedG[_rows];
	uint8_t wipedB[_rows];

	for (int i = 0; i < _cols + 9; i++) {
		if (i > 0) {
			// restore previous wiped line
			for (int yy = 0; yy < _rows; yy++) {
				SetLedInFrame(CURRFAME_IDX, yy, i-1, wipedR[yy], wipedG[yy], wipedB[yy]);
			}
		}
		for (int x = 0; x < _cols; x++) {
			for (int y = 0; y < _rows; y++){
				uint8_t r, g, b;
				GetLedFromFrame(CURRFAME_IDX, y, x, &r, &g, &b);
				if (x == i) {
					uint32_t maxcol = r > g ? r : g;
					maxcol = maxcol > b ? maxcol : b;
					if (maxcol > 0) {
						uint32_t factor = (255 * 1000) / maxcol;
						wipedR[y] = (r * factor) / 1000;
						wipedG[y] = (g * factor) / 1000;
						wipedB[y] = (b * factor) / 1000;
						SetLedInFrame(CURRFAME_IDX, y, x, wipedR[y], wipedG[y], wipedB[y]);
					} else {
						wipedR[y] = 0;
						wipedG[y] = 0;
						wipedB[y] = 0;
						SetLedInFrame(CURRFAME_IDX, y, x, RGB_FROM_SETTING);
					}
				} else if (x < i) {
					r /= 2;
					g /= 2;
					b /= 2;
					SetLedInFrame(CURRFAME_IDX, y, x, r, g, b);
				}
			}
		}
		DisplayFrame(CURRFAME_IDX);
		Sleep(50);
	}
	DisplayFrame(NEXTFAME_IDX);

}

static void RainbowSetColor(uint16_t nrOfActiveLeds, uint16_t currentLedIndex, uint8_t* r, uint8_t* g, uint8_t* b)
{
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.

	uint16_t gIdx = nrOfActiveLeds / 3;       
	uint16_t bIdx = (nrOfActiveLeds * 2) / 3;
	uint16_t rIdx = nrOfActiveLeds;

	if (nrOfActiveLeds < 3) {
		*r = 255; 
		*g = 0;
		*b = 0;
	} else if (currentLedIndex <= gIdx) {
		// Color between r and g
		*r = 255 * (gIdx - currentLedIndex) / gIdx; 
		*g = 255 * currentLedIndex / gIdx;
		*b = 0;
	} else if (currentLedIndex <= bIdx) {
		// Color between g and b. Shift gIdx to zero
		currentLedIndex -= gIdx;
		bIdx -= gIdx;
		*r = 0;
		*g = 255 * (bIdx - currentLedIndex) / bIdx; 
		*b = 255 * currentLedIndex / bIdx;
	}else {
		// Color between b and r. Shift bIdx to zero
		currentLedIndex -= bIdx;
		rIdx -= bIdx;
		*g = 0;
		*b = 255 * (rIdx - currentLedIndex) / rIdx; 
		*r = 255 * currentLedIndex / rIdx;
	}

	*r = ApplyBrightness(*r);
	*g = ApplyBrightness(*g);
	*b = ApplyBrightness(*b);

}

static uint16_t getNrOfActiveLeds(void)
{
    uint8_t row, col;
    uint8_t r, g, b;
    uint16_t nrOfLedsOn = 0;

    for (row = 0; row < _rows; row++) {
        for (col = 0; col < _cols; col++) {
            GetLedFromFrame(NEXTFAME_IDX, row, col, &r, &g, &b);
            if ((r + g + b) > 0) {
                nrOfLedsOn ++;
            }
        }
    }
    return nrOfLedsOn;
}

static void FilterRainbow(void)
{
    uint8_t row, col;
    uint8_t r, g, b;
    uint16_t nrOfActiveLeds = getNrOfActiveLeds();
    uint16_t currentLed = 0;
    for (row = 0; row < _rows; row++) {
        for (col = 0; col < _cols; col++) {
            GetLedFromFrame(NEXTFAME_IDX, _rows - 1 - row, col, &r, &g, &b);
            if ((r + g + b) > 0) {
                RainbowSetColor(nrOfActiveLeds, currentLed, &r, &g, &b);
                SetLedInFrame(NEXTFAME_IDX, _rows - 1 - row, col, r, g, b);
                currentLed ++;
            }
        }
    }
}

void AlsApplyTextEffect(ETextEffect filter)
{
	static int rnd_last = 0;
	uint32_t h, m, s;		

    switch(filter) {
    case TEXTEFFECT_NONE:
        break;
    case TEXTEFFECT_RAINBOW:
        FilterRainbow();
        break;
    case TEXTEFFECT_RANDOM:
        TimeGet(&h, &m, &s);
        if (h != rnd_last) {
            rnd_last = h;
            SettingRandomColors();
        }
        break;
    default:
        break;
    }
}

void AlsRefresh(TAlsEffects effect)
{ 

	if (effect == ALSEFFECT_RANDOM_EFFECT) {
		effect = rand() % 6 + ALSEFFECT_RANDOM_EFFECT + 1;
	}
	switch(effect) {
	case ALSEFFECT_NONE:
		// skip write when no change
		DisplayFrame(NEXTFAME_IDX);
		break;
				
	case ALSEFFECT_SHIFTRIGHT:
		RefreshRight(TRUE, 10 + rand() % 100);
		break;
		
	case ALSEFFECT_SHIFTDOWN:
		RefreshUp(TRUE, 10 + rand() % 100);		
		break;

	case ALSEFFECT_ZIP:
		RefreshZip(TRUE, 300);		
		break;

	case ALSEFFECT_FADE:
		RefreshFade();
		break;

	case ALSEFFECT_BLEND:
		RefreshBlend();
		break;
		
	case ALSEFFECT_MATRIX:
		break;

	case ALSEFFECT_WIPEDOWN:
	case ALSEFFECT_WIPERIGHT:
		RefreshWipe();
		break;

	
	default:
		DisplayFrame(NEXTFAME_IDX);
		break;
	}
	// memcpy(_currFrame, _nextFrame, sizeof(_currFrame));
}


void AlsSetRandom(uint8_t brightness) {
	uint8_t red = 0, green = 0, blue = 0;
	uint32_t row = rand() % _rows;
	uint32_t col = rand() % _cols;
	
	AlsGetLed(row, col, &red, &green, &blue);
	
	if (red != 0 || green != 0 || blue != 0) {
		AlsSetLed(row, col, 0,0,0);
	} else {
		AlsSetLed(row, col, rand() % brightness, rand() % brightness, rand() % brightness);
	}
	
}
