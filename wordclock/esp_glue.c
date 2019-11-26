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

#include <event_handler.h>
#include <buttons.h>
#include <hier_ben_ik.h>
#include <wordclock_main.h>
#include <settings.h>

static bool _isInterrupted = false;

uint32_t GetTicksDiffMs(uint32_t start, uint32_t end) {
    uint32_t diff = end-start;
    return diff * portTICK_RATE_MS;
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
		vTaskDelay(1 + (ms) / portTICK_RATE_MS);
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

//function is not used any more
void HexDump (char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			buff[i % 16] = '.';
		else
			buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
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

    struct sdk_station_config config = {"", "", 0, {0}};

	if (!sdk_wifi_station_get_config(&config)) {
		printf("ERROR sdk_wifi_station_get_config\n");
	}
	printf("wifi config: ssid:%s, pw:%s.\n", config.ssid, config.password);
//	if (strlen(config.ssid) == 0) {
//		strncpy((char*) config.ssid, "default", sizeof(config.ssid));
//		strncpy((char*) config.password, "default", sizeof(config.password));
//		sdk_wifi_set_opmode(STATION_MODE);
//		if (!sdk_wifi_station_set_config(&config)) {
//			printf("ERROR sdk_wifi_station_set_config\n");
//		}
//		printf("No wifi config. Written default wifi config\n");
//        vTaskDelay(1000 / portTICK_RATE_MS);
//        sdk_system_restart();
//	}

	if (!sdk_wifi_set_opmode(STATION_MODE)){
		printf("Error sdk_wifi_set_opmode\n");
	}
	sdk_wifi_station_set_auto_connect(TRUE);
//	sdk_wifi_station_connect();

	EvtHdlInit();

	ButtonsInit();
    char* ssidRobert="Robert";
    char* ssidRutger="Sjormie";
    if (strncasecmp(ssidRobert, (char*)config.ssid, strlen(ssidRobert)) == 0) {
        ownerOfClock = USER_ROBERT_WASSENS;
		HbiInit();
    } else if (strncmp(ssidRutger, (char*)config.ssid, strlen(ssidRutger)) == 0) {
        ownerOfClock = USER_RUTGER_HUIJGEN;
        HbiInit();
    } else {
	    ownerOfClock = USER_GUEST;
	}

	xTaskCreate(WordclockMain, (signed char *)"Main task", 1024, NULL, 1, NULL);
}
