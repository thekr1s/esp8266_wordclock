/*
 * font.c
 *
 *  Created on: Jun 21, 2015
 *      Author: robert.wassens
 */
#include "font.h"


static TSetPixelFunction _SetPixel;

void FontInit(TSetPixelFunction func){
	_SetPixel = func;
}

/**
 * Draw the font Top Down, row for row
 * @param rows
 * @param cols
 * @param row
 * @param col
 * @param pixels
 * @param r
 * @param g
 * @param b
 */
void FontPutCharTD(uint8_t rows, uint8_t cols, uint32_t row, uint32_t col, const uint8_t pixels[], uint8_t red, uint8_t green, uint8_t blue) {
	int r, c;
	
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			if ((pixels[r] >> c) & 0x1) {
				_SetPixel(row + rows - 1 - r, col + cols - 1 - c, red, green, blue);
			} 
		}
	}
	
}

/**
 * Draw the font Left to Right, column for column
 * @param rows
 * @param cols
 * @param row
 * @param col
 * @param pixels
 * @param r
 * @param g
 * @param b
 */
void FontPutCharLR(uint8_t rows, uint8_t cols, uint32_t row, uint32_t col, const uint8_t pixels[], uint8_t red, uint8_t green, uint8_t blue) {
	int c, r;
	
	for (c = 0; c < cols; c++) {
		for (r = 0; r < rows; r++) {
			if ((pixels[c] >> r) & 0x1) {
				_SetPixel(row + rows - 1 - r, col + c, red, green, blue);
			} 
		}
	}
	
}

/**
 * Draw bits in a line HORIZONTAL or VERTICAL. 
 */
void FontLine(uint8_t row, uint8_t col, uint32_t bits, bool horizontal, uint8_t r, uint8_t g, uint8_t b) {
	int i;
	int firstOne = 31;
	while(firstOne > 0 && !((bits >> firstOne) & 1) ) {
		firstOne--;
	}
	//assert (firstOne > 0);
	if (firstOne == -1) {
		return;
	}
	if (horizontal) {
		for (i = 0; i <= (firstOne); i++){
			if ((bits >> (firstOne - i)) & 1) {
				_SetPixel(row, col + i, r, g, b);
			}
		}
	} else {
		for (i = 0; i <= (firstOne); i++){
			if ((bits >> (firstOne - i)) & 1) {
				_SetPixel(row + i, col, r, g, b);
			}		
		}
		
	}
}
