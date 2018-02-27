/*
 * clock_words.h
 *
 *  Created on: Jul 5, 2015
 *      Author: robert.wassens
 */

#ifndef CLOCK_WORDS_H_
#define CLOCK_WORDS_H_

#include "stdbool.h"

#define TRUE true
#define FALSE false

void CWInit(uint32_t rows, uint32_t cols);
void CWSet(const char* word, uint8_t r, uint8_t g, uint8_t b);
void CWDisplayTime(uint32_t hours, uint32_t minutes, uint8_t r, uint8_t g, uint8_t b);
void CWDisplayAccurateTime(uint32_t hours, uint32_t minutes,  uint32_t seconds, uint8_t r, uint8_t g, uint8_t b);

#endif /* CLOCK_WORDS_H_ */
