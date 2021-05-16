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

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif


uint32_t GetTicksDiffMs(uint32_t start, uint32_t end);
void SetInterrupted(bool isInterrupted);
bool Interrupted();
uint32_t Sleep(uint32_t ms);
void SleepNI(uint32_t ms);
void TimeGet(uint32_t* h, uint32_t* m, uint32_t* s);

void user_init(void);

#endif //ESP_GLUE_H_
