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
#include <sysparam.h>

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

void test_sysparam() {
    char test[20];
    char *pTtest = test;
    bool result = true;
    printf("Check sysparam\n");
    if (sysparam_set_string("test", "dummy") != SYSPARAM_OK) {
        printf("Failed to program test string to sysparam\n");
        result = false;
    }
    if (sysparam_get_string("test", &pTtest) != SYSPARAM_OK) {
        printf("Failed to get test sysparam\n");
        result = false;
    }
    if (strcmp(pTtest, "dummy") != 0) {
        printf("Failed to read back sysparam\n");
        result = false;
    }
    
    if (!result) {
        uint32_t sysparam_addr;
        sysparam_status_t status;
        sysparam_addr = sdk_flashchip.chip_size - (5 + DEFAULT_SYSPARAM_SECTORS) * sdk_flashchip.sector_size;
        status = sysparam_create_area(sysparam_addr, DEFAULT_SYSPARAM_SECTORS, true);
        if (status == SYSPARAM_OK) {
            printf("Sysparam create area succeeded\n");
        } else {
            printf("Failed to create Sysparam area: %d\n", status);
        }
        printf("Sysparam is cleared, reboot in 2 seconds\n");
        vTaskDelay(2 * 1000);
        sdk_system_restart();
    }
}

/*
 * This function is called from app_main.c within the esp_rtos,
 * this is where the system starts
 */
void user_init(void)
{
	gpio_set_iomux_function(2, IOMUX_GPIO2_FUNC_UART1_TXD);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    uart_set_baud(0, 115200);
    printf("--- RMW Wordclock ---\r\n");

    if (!sdk_wifi_set_opmode(STATION_MODE)){
		printf("Error sdk_wifi_set_opmode\n");
	}
	sdk_wifi_station_set_auto_connect(TRUE);

    // for (int i =0; i < 2; i++) {
    //     vTaskDelay(1000);
    //     printf("Wait\n");
    // }
    test_sysparam();

	//Low level init
    SettingsInit();
	wordClockDisplay_init();
	ButtonsInit();

    //Start tasks
	EvtHdlInit();
    HbiInit();
    sntp_client_init();
    wificfg_init();
	xTaskCreate(WordclockMain, "Main task", 1024, NULL, 1, NULL);
}
