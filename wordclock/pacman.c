/*
 * pacman.c
 *
 *  Created on: Jun 21, 2015
 *      Author: robert.wassens
 */
#include "font.h"

static const uint8_t pacman[][5] = {
	{0xE, 0x1F, 0x1F, 0x1F, 0xE},
	{0xE, 0x1F, 0x1c, 0x1F, 0xE},
	{0xE, 0x1C, 0x18, 0x1C, 0xE},
	{0xE, 0x1F, 0x1c, 0x1F, 0xE},
};

static const uint8_t ghost[][5] = {
	{0xE,0x1F,0x15,0x1F,0x15},
	{0xE,0x15,0x1F,0x1F,0x15},
};


void PCPutPC(uint32_t row, uint32_t col, uint8_t r, uint8_t  g, uint8_t  b) {
	static int idx = 0;
	
	FontPutCharTD(5,5,row,col, &pacman[idx][0], r, g, b);
	idx++;
	idx %= 4;
}

void PCPutGhost(uint32_t row, uint32_t col, uint8_t r, uint8_t  g, uint8_t  b) {
	static int idx = 0;
	
	FontPutCharTD(5,5,row,col, &ghost[idx/2][0], r, g, b);
	idx++;
	idx %= 4;
}
