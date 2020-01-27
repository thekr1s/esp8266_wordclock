/*
 * Test code for SNTP on esp-open-rtos.
 *
 * Jesus Alonso (doragasu)
 */
#include <espressif/esp_common.h>
#include <esp/uart.h>

#include <string.h>
#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>

#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <lwip/dns.h>

/* Add extras/sntp component to makefile for this include to work */
#include <sntp.h>
#include <time.h>


#include "event_handler.h"

#define SNTP_SERVERS 	"0.pool.ntp.org", "1.pool.ntp.org", \
						"2.pool.ntp.org", "3.pool.ntp.org"

#define vTaskDelayMs(ms)	vTaskDelay((ms)/portTICK_PERIOD_MS)
#define UNUSED_ARG(x)	(void)x
#define UPDATE_INERVAL (5 * 60000)
static struct timezone _tz = {0, 0};
static uint32_t _rtcTicsPerSec = 0;

void sntp_tsk(void *pvParameters)
{
	const char *servers[] = {SNTP_SERVERS};
	UNUSED_ARG(pvParameters);

	printf("SNTP: Wait for WiFi connection... \n");
	while(sdk_wifi_station_get_connect_status() != STATION_GOT_IP) {
		vTaskDelayMs(1000);
	}

	/* Start SNTP */
	printf("Starting SNTP... \n");
	/* SNTP will request an update each 15 minutes */
	sntp_set_update_delay(UPDATE_INERVAL);
	/* Set GMT+1 zone, daylight savings off. DST is handled in the wordclock app itself */
	/* SNTP initialization */
	sntp_initialize(&_tz);
	/* Servers must be configured right after initialization */
	sntp_set_servers(servers, sizeof(servers) / sizeof(char*));

	printf("Wait for NTP time...\n");
	while (time(NULL) < 10000) {
		vTaskDelayMs(1000);

	}
	printf("Time set...\n");

	/* Print date and time each 5 seconds */
	while(1) {
		const delaySec = 5;
		uint32_t t = RTC.COUNTER;
		vTaskDelayMs(delaySec * 1000);
		_rtcTicsPerSec = (RTC.COUNTER - t) / delaySec;		
//		time_t ts = time(NULL);
//		printf("TIME: %s", ctime(&ts));
	}
}

bool sntp_client_time_valid() {
	// TODO: implement
	if (_rtcTicsPerSec == 0) return false;

	// sntp_fun.c stores the last update in RTC scratch2 register. See line:
	//     tim_ref = now_rtc;
	// At the bottom of the c file.
	// Use that to calculate the age of the time update.
	uint32_t age = (RTC.COUNTER - RTC.SCRATCH[2]) / _rtcTicsPerSec;
	return ((RTC.COUNTER - RTC.SCRATCH[2]) / _rtcTicsPerSec <= (2 * UPDATE_INERVAL) / 1000)? true : false;
}

void sntpClientIinit(const struct timezone* tz)
{
	_tz = *tz;
     xTaskCreate(sntp_tsk, "SNTP", 1024, NULL, 1, NULL);
}

