/*
 * font3x5.h
 *
 *  Created on: Jun 19, 2015
 *      Author: robert.wassens
 */

#ifndef FONT3X5_H_
#define FONT3X5_H_

#include <stdint.h>
#include <stdbool.h>
#include "font.h"



void F3x5Init(TSetPixelFunction func);

void F3x5WriteChar(uint32_t row, uint32_t col, char c, uint8_t r, uint8_t g, uint8_t b);


#endif /* FONT3X5_H_ */
