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

#define WORDCLOCKMAIN_TASK_PRIO 1
#define WIFI_CONFIG_TASK_PRIO 2
#define TFTP_CLIENT_TASK_PRIO 2     //tftp client is started contitional
#define SNTP_SERVER_TASK_PRIO 1
#define EVENT_TASK_PRIO 1
#define HIERBENIK_TASK_PRIO 1


uint32_t GetTicksDiffMs(uint32_t start, uint32_t end);
void SetInterrupted(bool isInterrupted);
bool Interrupted();
uint32_t Sleep(uint32_t ms);
void SleepNI(uint32_t ms);

void user_init(void);


#endif //ESP_GLUE_H_
