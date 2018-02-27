/*
 * font4x5.h
 *
 *  Created on: Jun 19, 2015
 *      Author: robert.wassens
 */

#ifndef FONT4X5_H_
#define FONT4X5_H_

#include <stdint.h>
#include <stdbool.h>
#include "font.h"


void F4x5WriteChar(uint32_t row, uint32_t col, char c, uint8_t r, uint8_t g, uint8_t b);


#endif /* FONT4X5_H_ */
