/*
 * font4x5.h
 *
 *  Created on: Jun 19, 2015
 *      Author: robert.wassens
 */

#ifndef FONT5X7_H_
#define FONT5X7_H_

#include <stdint.h>
#include <stdbool.h>
#include "font.h"


void F5x7WriteChar(uint32_t row, uint32_t col, char c, uint8_t r, uint8_t g, uint8_t b);


#endif /* FONT5X7_H_ */
