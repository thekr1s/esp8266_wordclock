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


// See the definition of "ws2812_pixel_t"
#define ALS_BYTES_PER_LED  4
#define BLUE_IDX 	0
#define GREEN_IDX 	1
#define RED_IDX 	2
#define WHITE_IDX 	3

typedef uint8_t TPixel[ALS_BYTES_PER_LED];

typedef void (*TAlsWriteFunction)(TPixel* pData, uint32_t byteCount);

void AlsInit(uint32_t rows, uint32_t cols, TAlsWriteFunction writeFunction); 
uint32_t AlsGetRows();
uint32_t AlsGetCols();
void AlsSetBackgroundColor(uint8_t red, uint8_t green, uint8_t blue);
void AlsFill(uint8_t red, uint8_t green, uint8_t blue);
void AlsSetLed(uint32_t row, uint32_t col, uint8_t red, uint8_t green, uint8_t blue);
void AlsGetLed(uint32_t row, uint32_t col, uint8_t* pRed, uint8_t* pGreen, uint8_t* pBlue);
void AlsApplyTextEffect(ETextEffect filter);
void AlsRefresh(TAlsEffects effect);

void AlsSetRandom(uint8_t brightness);

#endif /* ADDRESSABLELEDSTRIP_H_ */
