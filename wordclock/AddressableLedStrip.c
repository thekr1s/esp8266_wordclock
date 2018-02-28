/*
 * LedStrip.cpp
 *
 *  Created on: May 9, 2015
 *      Author: robert.wassens
 */
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "esp_glue.h"
#include "AddressableLedStrip.h"
#include "displaySettings.h"
#include "settings.h"


#define ALS_BYTES_PER_LED  3
#define ALS_MAX_LED_COUNT  (WORDCLOCK_ROWS_MAX * WORDCLOCK_COLLS_MAX)

static uint8_t _redIdx   = 0;
static uint8_t _greenIdx = 1;
static uint8_t _blueIdx  = 2;
static bool    _flipCols = FALSE;


uint8_t _nextFrame[ALS_MAX_LED_COUNT ][ALS_BYTES_PER_LED];
uint8_t _currFrame[ALS_MAX_LED_COUNT ][ALS_BYTES_PER_LED];

static uint32_t _rows = 0;
static uint32_t _cols = 0;
TAlsWriteFunction _writeFunction = NULL;

extern uint32_t TimerTickCount;
extern uint8_t g_brightness;

void AlsInit(uint32_t rows, uint32_t cols, TAlsWriteFunction writeFunction, 
		uint8_t redIdx, uint8_t greenIdx, uint8_t blueIdx, bool flipCols) 
{
	_writeFunction = writeFunction;
	_rows = rows;
	_cols = cols;

	_redIdx   = redIdx;
	_greenIdx = greenIdx;
	_blueIdx  = blueIdx;
	_flipCols = flipCols;

}

uint32_t AlsGetRows() {
	return _rows;
}

uint32_t AlsGetCols(){
	return _cols;
}

void AlsFill(uint8_t red, uint8_t green, uint8_t blue){
	int idx;
	for (idx = 0; idx < _cols * _rows; idx++) {
	
		_nextFrame[idx][_redIdx] = red;
		_nextFrame[idx][_greenIdx] = green;
		_nextFrame[idx][_blueIdx] = blue;
	}	
}

static void SetLedInFrame(uint8_t frame[][ALS_BYTES_PER_LED], uint32_t row, uint32_t col, uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t idx;
	
	if (_flipCols) {
		col = _cols - 1 - col;
	}
	
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
	frame[idx][_redIdx] = r;
	frame[idx][_greenIdx] = g;
	frame[idx][_blueIdx] = b;

}

void AlsSetLed(uint32_t row, uint32_t col, uint8_t red, uint8_t green, uint8_t blue)
{
	SetLedInFrame(_nextFrame, row, col, red, green, blue);
}

static void GetLedFromFrame(uint8_t frame[][ALS_BYTES_PER_LED], uint32_t row, uint32_t col, uint8_t* r, uint8_t* g, uint8_t* b)
{
	if (row >= _rows || col >= _cols) {
		//Ignore calls when index out of range
		return; 
	}

	if (_flipCols) {
		col = _cols - 1 - col;
	}

	uint32_t idx =  (_rows - 1 - row) * _cols;

	if (row % 2 == 0) {
		idx += col;
	} else {
		idx += _cols - 1 - col;		
	}
	*r = frame[idx][_redIdx];
	*g = frame[idx][_greenIdx];
	*b = frame[idx][_blueIdx];	
}

void AlsGetLed(uint32_t row, uint32_t col, uint8_t* pRed, uint8_t* pGreen, uint8_t* pBlue)
{
	GetLedFromFrame(_nextFrame, row, col, pRed, pGreen, pBlue);
}

static void ShiftHorizontal(bool shiftRight)
{
	uint8_t r,g,b;
	int32_t col, row;
	
	for (col = _cols - 1; col >= 0; col--) {
		for(row = 0; row < _rows; row++) {
			if (col >= 0){
				GetLedFromFrame(_currFrame, row, col - 1, &r, &g, &b);
				SetLedInFrame(_currFrame, row, col, r, g, b);
			} else {
				SetLedInFrame(_currFrame, row, col, BGRGB_FROM_SETTING);
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
				GetLedFromFrame(_currFrame, row + 1, col, &r, &g, &b);
				SetLedInFrame(_currFrame, row, col, r, g, b);
			} else {
				SetLedInFrame(_currFrame, row, col, BGRGB_FROM_SETTING);
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
				GetLedFromFrame(_nextFrame, row, _cols - i - 1, &r, &g, &b);
				SetLedInFrame(_currFrame, row, 0, r, g, b);
				
			}
		}
		_writeFunction(&_currFrame[0][0], _cols * _rows * ALS_BYTES_PER_LED);
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
				GetLedFromFrame(_nextFrame, i, col, &r, &g, &b);
				SetLedInFrame(_currFrame, _rows - 1, col, r, g, b);
				
			}
		}
		_writeFunction(&_currFrame[0][0], _cols * _rows * ALS_BYTES_PER_LED);
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
				GetLedFromFrame(_currFrame, row, col, &r, &g, &b);
				r /= 2;
				g /= 2;
				b /= 2;
				if (r != 0 || g != 0 || b != 0) {
					done = FALSE;
				}
				SetLedInFrame(_currFrame, row, col, r, g, b);
			}
			_writeFunction(&_currFrame[0][0], _cols * _rows * ALS_BYTES_PER_LED);
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
			GetLedFromFrame(_currFrame, row, col, &r, &g, &b);
			GetLedFromFrame(_currFrame, row + 1, col, &r1, &g1, &b1);
			r = Zip(r, r1);
			g = Zip(g, g1);
			b = Zip(b, b1);
			SetLedInFrame(_currFrame, row + 1, col, r, g, b);
			SetLedInFrame(_currFrame, row, col, 0, 0, 0);
			row = _rows - c - 1;
			
			GetLedFromFrame(_currFrame, row, col, &r, &g, &b);
			GetLedFromFrame(_currFrame, row - 1, col, &r1, &g1, &b1);
			r = Zip(r, r1);
			g = Zip(g, g1);
			b = Zip(b, b1);
			SetLedInFrame(_currFrame, row - 1, col, r, g, b);
			SetLedInFrame(_currFrame, row, col, 0, 0, 0);
		}
		_writeFunction(&_currFrame[0][0], _cols * _rows * ALS_BYTES_PER_LED);
		Sleep(delay);
		if (delay > 120) {
			delay /= 2;
		}
	}
	for (col = 0; col < _cols; col++) {
		GetLedFromFrame(_currFrame, centre, col, &r, &g, &b);
		r = r < 64 ? r * 4 : 255;
		g = g < 64 ? g * 4 : 255;
		b = b < 64 ? b * 4 : 255;
		SetLedInFrame(_currFrame, centre, col, r, g, b);
	}
	_writeFunction(&_currFrame[0][0], _cols * _rows * ALS_BYTES_PER_LED);
	Sleep(100);

	for (c = 0; c < 10; c++) {
		for (col = 0; col < _cols; col++) {
			GetLedFromFrame(_currFrame, centre, col, &r, &g, &b);
			r /= 2;
			g /= 2;
			b /= 2;
			SetLedInFrame(_currFrame, centre, col, r, g, b);
		}
		_writeFunction(&_currFrame[0][0], _cols * _rows * ALS_BYTES_PER_LED);
		Sleep(100);	    	
	}
	if(displayNext) {
		_writeFunction(&_nextFrame[0][0], _cols * _rows * ALS_BYTES_PER_LED);
		
	}
}

static void RefreshBlend(void)
{
	
}

static void RefreshFade(void)
{
	FadeOut();
	_writeFunction(&_nextFrame[0][0], _cols * _rows * ALS_BYTES_PER_LED);

}


void AlsRefresh(TAlsEffects effect)
{ 
	if (effect == ALSEFFECT_RANDOM_EFFECT) {
		effect = rand() % 4 + ALSEFFECT_RANDOM_EFFECT + 1;
	}
	switch(effect) {
	case ALSEFFECT_NONE:
		// skip write when no change
		_writeFunction(&_nextFrame[0][0], _cols * _rows * ALS_BYTES_PER_LED);
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
		
	
	default:
		_writeFunction(&_nextFrame[0][0], _cols * _rows * ALS_BYTES_PER_LED);
		break;
	}
	memcpy(_currFrame, _nextFrame, sizeof(_currFrame));
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