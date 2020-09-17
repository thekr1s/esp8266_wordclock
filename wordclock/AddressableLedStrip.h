/*
 * AddressableLedStrip.h
 *
 *  Created on: May 9, 2015
 *      Author: robert.wassens
 */

#ifndef ADDRESSABLELEDSTRIP_H_
#define ADDRESSABLELEDSTRIP_H_

#include <sys/types.h>
#include <stdint.h>

typedef enum {
	ALSEFFECT_NONE,
	ALSEFFECT_RANDOM_EFFECT,
	ALSEFFECT_SHIFTRIGHT,
	ALSEFFECT_SHIFTDOWN,	
	ALSEFFECT_ZIP,	
	ALSEFFECT_FADE,
	ALSEFFECT_WIPERIGHT,
	ALSEFFECT_WIPEDOWN,
	ALSEFFECT_BLEND,
	ALSEFFECT_MATRIX,
} TAlsEffects;

typedef enum {
    TEXTEFFECT_NONE,
    TEXTEFFECT_RAINBOW,
    TEXTEFFECT_RANDOM,
    TEXTEFFECT_MAX,
} ETextEffect;

typedef void (*TAlsWriteFunction)(uint8_t* pData, uint32_t byteCount);

void AlsInit(uint32_t rows, uint32_t cols, TAlsWriteFunction writeFunction, 
		uint8_t redIdx, uint8_t greenIdx, uint8_t blueIdx, bool flipCols); 
uint32_t AlsGetRows();
uint32_t AlsGetCols();
void AlsFill(uint8_t red, uint8_t green, uint8_t blue);
void AlsSetLed(uint32_t row, uint32_t col, uint8_t red, uint8_t green, uint8_t blue);
void AlsGetLed(uint32_t row, uint32_t col, uint8_t* pRed, uint8_t* pGreen, uint8_t* pBlue);
void AlsApplyTextEffect(ETextEffect filter);
void AlsRefresh(TAlsEffects effect);

void AlsSetRandom(uint8_t brightness);

#endif /* ADDRESSABLELEDSTRIP_H_ */
