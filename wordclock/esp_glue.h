/*
 * esp_glue.h
 *
 *  Created on: Oct 7, 2016
 *      Author: rutger.huijgen
 */

#ifndef ESP_GLUE_H_
#define ESP_GLUE_H_
#include "stdint.h"
#include "stdbool.h"

#define TRUE true
#define FALSE false

uint32_t GetTicksDiffMs(uint32_t start, uint32_t end);
void SetInterrupted(bool isInterrupted);
bool Interrupted();
void Sleep(uint32_t ms);
void SleepNI(uint32_t ms);

void user_init(void);












#endif //ESP_GLUE_H_