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

#include <ssid_config.h>

/* Add extras/sntp component to makefile for this include to work */
#include <sntp.h>
#include <time.h>


#include "event_handler.h"

#define SNTP_SERVERS 	"0.pool.ntp.org", "1.pool.ntp.org", \
						"2.pool.ntp.org", "3.pool.ntp.org"

#define vTaskDelayMs(ms)	vTaskDelay((ms)/portTICK_RATE_MS)
#define UNUSED_ARG(x)	(void)x
#define UPDATE_INERVAL (5 * 60000)
static struct timezone _tz = {0, 0};

void sntp_tsk(void *pvParameters)
{
	char *servers[] = {SNTP_SERVERS};
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
		vTaskDelayMs(5000);
//		time_t ts = time(NULL);
//		printf("TIME: %s", ctime(&ts));
	}
}

bool sntp_client_time_valid() {
	// TODO: implement
	return (sntp_get_last_update_age_sec() <= (2 * UPDATE_INERVAL) / 1000)? true : false;
}

void sntpClientIinit(const struct timezone* tz)
{
	_tz = *tz;
     xTaskCreate(sntp_tsk, (signed char *)"SNTP", 1024, NULL, 1, NULL);
}

