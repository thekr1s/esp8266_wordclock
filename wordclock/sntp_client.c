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

#define SNTP_SERVERS 	"0.pool.ntp.org", "1.pool.ntp.org", \
						"2.pool.ntp.org", "3.pool.ntp.org"

#define vTaskDelayMs(ms)	vTaskDelay((ms)/portTICK_PERIOD_MS)
#define UNUSED_ARG(x)	(void)x
#define UPDATE_INERVAL (5 * 60000)

static struct timezone _tz = {1*60, 0};
static volatile uint32_t _rtcTicsPerSec = 0;
static const char *servers[] = {SNTP_SERVERS};

void sntp_tsk(void *pvParameters)
{
	UNUSED_ARG(pvParameters);

	printf("SNTP: Wait for WiFi connection... \n");
	while (sdk_wifi_station_get_connect_status() != STATION_GOT_IP) {
		printf("SNTP: Wait for WiFi connection... \n");
		vTaskDelayMs(2000);
	}

	printf("Wait for NTP time...\n");
	while (time(NULL) < 10000) {
		vTaskDelayMs(1000);

	}
	printf("Time set...\n");

	/* Print date and time each 5 seconds */
	while(1) {
		const uint32_t delaySec = 5;
		uint32_t t = RTC.COUNTER;
		vTaskDelayMs(delaySec * 1000);
		_rtcTicsPerSec = (RTC.COUNTER - t) / delaySec;		
		// time_t ts = time(NULL);
		// printf("TIME: %s", ctime(&ts));
	}
}

bool sntp_client_time_valid(void) {
	if (_rtcTicsPerSec == 0) return false;

	uint32_t age = time(NULL) - sntp_last_update_ts();
	return (age <= (2 * UPDATE_INERVAL) / 1000)? true : false;
}

void sntp_client_init(void)
{
	printf("Starting SNTP... \n");

	sntp_set_update_delay(UPDATE_INERVAL);
	sntp_set_servers(servers, sizeof(servers) / sizeof(char*));
	sntp_initialize(&_tz);
    
	xTaskCreate(sntp_tsk, "SNTP", 256, NULL, 1, NULL);
}
