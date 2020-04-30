/*
 * esp_glue.c
 *
 *  Created on: Oct 7, 2016
 *      Author: rutger.huijgen
 */

#include "FreeRTOS.h"
#include "task.h"

#include "espressif/esp_common.h"
#include <espressif/esp_wifi.h>
#include "esp/uart.h" // uart_set_baud

#include <stdio.h> // printf
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#include <esp_glue.h>
#include <event_handler.h>
#include <buttons.h>
#include <hier_ben_ik.h>
#include <wificfg.h>
#include <sntp_client.h>
#include <wordclock_main.h>
#include <settings.h>

static volatile bool _isInterrupted = false;

uint32_t GetTicksDiffMs(uint32_t start, uint32_t end) {
    uint32_t diff = end-start;
    return diff * portTICK_PERIOD_MS;
}

void SetInterrupted(bool isInterrupted) {
//	printf("%s %d %d\n", __FUNCTION__, isInterrupted, _isInterrupted);
	_isInterrupted = isInterrupted;
}

bool Interrupted() {
    return _isInterrupted;
}

/**
 * Sleep Non Interruptable
 */
void SleepNI(uint32_t ms) {
	if (ms == 0) {
		vTaskDelay(0);
	} else {
		vTaskDelay(1 + (ms) / portTICK_PERIOD_MS);
	}
}

/**
 * Sleep interruptable by buttonpress or call to SetInterrupted()
 * 
 * Returns: number of ms left when interrupted. 0 when not interrupted
 */
uint32_t Sleep(uint32_t ms) {
    while ( ms > 0) {
		if (_isInterrupted) return ms;
		SetInterrupted(ButtonsAnyPressed());
		SleepNI(ms > 100 ? 100 : ms);
		ms = ms > 100 ? ms - 100 : 0;
	}
    return ms;
}

/*
 * This function is called from app_main.c within the esp_rtos,
 * this is where the system starts
 */
void user_init(void)
{
	sdk_wifi_station_set_auto_connect(FALSE);
	gpio_set_iomux_function(2, IOMUX_GPIO2_FUNC_UART1_TXD);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    uart_set_baud(0, 115200);
    printf("--- RMW Wordclock ---\r\n");

	SettingsInit();
	wordClockDisplay_init();
	ButtonsInit();

	EvtHdlInit();
	sntp_client_init();
    HbiInit();
    wificfg_init();
    
    // This priority should be lower than wifi config and sntp.
	xTaskCreate(WordclockMain, "Main task", 1024, NULL, WORDCLOCKMAIN_TASK_PRIO, NULL);
}
