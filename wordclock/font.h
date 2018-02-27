/*
 * font3x5.h
 *
 *  Created on: Jun 19, 2015
 *      Author: robert.wassens
 */

#ifndef FONT_H_
#define FONT_H_

#include <stdint.h>
#include <stdbool.h>


typedef void (*TSetPixelFunction)(uint32_t row, uint32_t col, uint8_t r, uint8_t g, uint8_t b);

#include "font3x5.h"
#include "font4x5.h"
#include "font5x7.h"

void FontInit(TSetPixelFunction func);
void FontPutCharTD(uint8_t rows, uint8_t cols, uint32_t row, uint32_t col, const uint8_t pixels[], uint8_t r, uint8_t g, uint8_t b);
void FontPutCharLR(uint8_t rows, uint8_t cols, uint32_t row, uint32_t col, const uint8_t pixels[], uint8_t red, uint8_t green, uint8_t blue);

void FontLine(uint8_t row, uint8_t col, uint32_t bits, bool horizontal, uint8_t r, uint8_t g, uint8_t b);
#endif /* FONT3X5_H_ */
