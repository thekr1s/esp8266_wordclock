/*
 * Test code for SNTP on esp-open-rtos.
 *
 * Jesus Alonso (doragasu)
 */
#include <espressif/esp_common.h>
#include <esp/uart.h>

#include <string.h>
#include <stdio.h>
#include <math.h>

#include <FreeRTOS.h>
#include <task.h>

#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <lwip/dns.h>

/* SNTP is a extra component, this is inculded in the makefile */
#include <sntp.h>
#include <time.h>
#include "settings.h"

#define vTaskDelayMs(ms)	vTaskDelay((ms)/portTICK_PERIOD_MS)
#define UNUSED_ARG(x)	(void)x
#define UPDATE_INERVAL (5 * 60000)

struct timezone _timeZone = {0, 0};
static time_t _startSummerTime, _endSummerTime;
static int _currentYear = 0;
static volatile uint32_t _rtcTicsPerSec = 0;

static void calculateSummertime(time_t* start, time_t* end) {
	time_t ts = time(NULL);
	struct tm *pTM = localtime(&ts);
	// This formule is from: https://webspace.science.uu.nl/~gent0113/wettijd/wt_text4d.htm
	int year = pTM->tm_year + 1900;
	int alg_f = (int)( floor((float)(5 * year) / 4.0) - floor((float)year/100.0) + floor((float)year/400.0) ) % 7;
	int startDay = 31 - ((alg_f + 5) % 7);
	int endDay = 31 - ((alg_f + 2) % 7);
	
	pTM->tm_hour = 1 + 1; //summer time starts UTC + 1 hour, And our timezone is +1
	pTM->tm_min = 0;
	pTM->tm_sec = 0;
	pTM->tm_year = year - 1900;
	pTM->tm_wday = 0;	//Always sunday
	
	// Start summer time is on martch == month number 2
	pTM->tm_mon = 2; 
	pTM->tm_mday = startDay;
	*start = mktime(pTM);
	printf("Start summer time: %s", asctime(localtime(start)));

	// End summer time is on oktober == month number 9
	pTM->tm_mon = 9; 
	pTM->tm_mday = endDay;
	pTM->tm_hour += 1; //also extra hour becuase we leave summer time
	*end = mktime(pTM);
	printf("End summer time: %s", asctime(localtime(end)));
}

static void correctDST() {
	int newDst = 0;

	time_t ts = time(NULL);
	struct tm *pTM = localtime(&ts);

	if (_currentYear != pTM->tm_year ) {
		_currentYear = pTM->tm_year;
		calculateSummertime(&_startSummerTime, &_endSummerTime);
	}

	if (g_settings.correctDST && ts > _startSummerTime && ts < _endSummerTime) {
		newDst = 1;
	}
	
	if (newDst != _timeZone.tz_dsttime || _timeZone.tz_minuteswest != g_settings.timeZoneOffsetMinuts) {
		_timeZone.tz_minuteswest = g_settings.timeZoneOffsetMinuts;
		_timeZone.tz_dsttime = newDst;
		printf("Changing timezone, timezone offset: %02d:%02d hours, DST is: %d \n", _timeZone.tz_minuteswest/60, abs(_timeZone.tz_minuteswest%60), _timeZone.tz_dsttime);
		sntp_set_timezone(&_timeZone); //Change is made after NTP update.
	}
}

bool sntp_client_time_valid(void) {
	if (_rtcTicsPerSec == 0) return false;

	uint32_t age = time(NULL) - sntp_last_update_ts();
	return (age <= (2 * UPDATE_INERVAL) / 1000)? true : false;
}

void sntp_tsk(void *pvParameters)
{
	UNUSED_ARG(pvParameters);

	while (sdk_wifi_station_get_connect_status() != STATION_GOT_IP) {
		printf("SNTP: Wait for WiFi connection... \n");
		vTaskDelayMs(2000);
	}

	printf("Wait for NTP time...\n");
	while (time(NULL) < 10000) {
		vTaskDelayMs(1000);
	}
	printf("Time set...\n");
	
	correctDST();
	sntp_set_update_delay(UPDATE_INERVAL);
	
	/* Print date and time each 5 seconds */
	while(1) {
		const uint32_t delaySec = 5;
		uint32_t t = RTC.COUNTER;
		vTaskDelayMs(delaySec * 1000);
		_rtcTicsPerSec = (RTC.COUNTER - t) / delaySec;	
		
		correctDST();
		
		// time_t ts = time(NULL);
		// printf("TIME: %s", ctime(&ts));
	}
}

void sntp_client_init(void)
{
	printf("Starting SNTP... \n");
	sntp_set_update_delay(UPDATE_INERVAL/60); //Increase the startup update interval
	sntp_initialize(&_timeZone);
	xTaskCreate(sntp_tsk, "SNTP", 256, NULL, 3, NULL);
}
