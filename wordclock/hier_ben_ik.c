/*
 * hier_ben_ik.c
 *
 *  Created on: Oct 27, 2016
 *      Author: robert
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include <math.h>
#include "espressif/esp_common.h"

#include "esp_glue.h"
#include "settings.h"

uint32_t _dist = 10000;
uint32_t _age = 10000;
float _lat = 0.0;
float _lon = 0.0;

void HbiGetDistAndAge(uint32_t* pDist, uint32_t* pAge) {
	*pDist = _dist;
	*pAge = _age;
}

void HbiGetLatLon(float* pLat, float* pLon) {
	*pLat = _lat;
	*pLon = _lon;
}

float deg2rad(float deg) {
  return deg * (M_PI/180);
}

float CalcDist(float lat1, float lon1, float lat2, float lon2) {
  float R = 6371; // Radius of the earth in km
  float dLat = deg2rad(lat2-lat1);  // deg2rad below
  float dLon = deg2rad(lon2-lon1); 
  float a = 
    sin(dLat/2) * sin(dLat/2) +
    cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * 
    sin(dLon/2) * sin(dLon/2)
    ; 
  float c = 2 * atan2(sqrt(a), sqrt(1-a)); 
  float d = R * c; // Distance in km
  return d;
}


static void ParseResponse(char* pResp){
	const char* ageTag = "age:";
	const char* distTag = "dist:";
	const char* latTag = "lat:";
	const char* lonTag = "lon:";
	char* p;
	int t, t2;
	float f;

	p = strstr(pResp, ageTag);
	if ( p && sscanf(p, "age:%d", &t) == 1) {
		_age = t;
	}
	p = strstr(pResp, distTag);
	if (p && sscanf(p, "dist:%d.%3d", &t, &t2) == 2) {
		_dist = t * 1000 + t2;
	}
	p = strstr(pResp, latTag);
	if (p && sscanf(p, "lat:%f", &f) == 1) {
		_lat = f;
	}
	p = strstr(pResp, lonTag);
	if (p && sscanf(p, "lon:%f", &f) == 1) {
		_lon = f;
		_dist = (1000.0 * CalcDist(_lat, _lon, g_settings.hierbenikHomeLat, g_settings.hierbenikHomeLon));
		printf("lat: %f, lon: %f, dist: %d, age: %d\n", _lat, _lon, _dist, _age);
	}
}

static void GetHierBenIk() {
	int successes = 0, failures = 0;
	printf("%s %d enter\n", __FUNCTION__, __LINE__);
	static struct sockaddr addr;
	static int addrLen = 0;
	int err;
	if (addrLen == 0) {
		struct addrinfo *res = NULL;
		const struct addrinfo hints = { .ai_family = AF_INET, .ai_socktype =
				SOCK_STREAM, };
		
		err = getaddrinfo(g_settings.hierbenikUrl, g_settings.hierbenikPort, &hints, &res);

		if (err != 0 || res == NULL ) {
			printf("DNS lookup failed (%s) err=%d res=%p\r\n", g_settings.hierbenikUrl, err, res);
			if (res)
				freeaddrinfo(res);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			failures++;
			return;
		}
		addr = *res->ai_addr;
		addrLen = res->ai_addrlen;
		freeaddrinfo(res);
	}
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		printf("... Failed to allocate socket.\r\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		failures++;
		return;
	}

	if (connect(s, &addr, addrLen) != 0) {
		closesocket(s);
		printf("... socket connect failed.\r\n");
		vTaskDelay(4000 / portTICK_PERIOD_MS);
		failures++;
		closesocket(s);
		addrLen = 0; // Resolve IP adress next time.
		return;
	}

	char tempRequest[76+MAX_URL_SIZE+MAX_URL_SIZE];
	sprintf(tempRequest, "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: Woordklok\r\nAccept: */*\r\n\r\n", g_settings.hierbenikRequest, g_settings.hierbenikUrl);
	err = lwip_write(s, tempRequest, strlen(tempRequest));
	
	if (err < 0) {
		printf("... socket send failed\r\n");
		closesocket(s);
		vTaskDelay(4000 / portTICK_PERIOD_MS);
		failures++;
		closesocket(s);
		addrLen = 0; // Resolve IP adress next time.
		return;
	}

	static char recv_buf[512];
	int r;
	int i = 0;
	bzero(recv_buf, sizeof(recv_buf));
	SleepNI(1000);
	r = lwip_read(s, &recv_buf[i], sizeof(recv_buf) - i - 1);
	i += r;
	ParseResponse(recv_buf);

	if (r != 0)
		failures++;
	else
		successes++;
	closesocket(s);
	// printf("%s %d leave, \n", __FUNCTION__, __LINE__);

}

void HbiTask(void *pvParameters){
	while (strcmp(g_settings.hierbenikUrl, "") == 0) {
		printf("Invalid Hier ben ik config, sleep task for 10 minutes\n");
		SleepNI(10*60*1000);
	}
	while (sdk_wifi_station_get_connect_status() != STATION_GOT_IP) {
		// printf("%s: Wait for connect\n", __FUNCTION__);
		SleepNI(5000);
	}
	// Apparently LWIP needs some time before DNS lookup to work. QaD workaround: sleep....
	SleepNI(5000);
	while (true) {
		GetHierBenIk();
		if (_age < 300) {
			SleepNI(10000);
		} else {
			SleepNI(60000);
		}

	}
}


void HbiInit() {
    xTaskCreate(HbiTask, "HierBenIk task", 1024, NULL, 2, NULL);
}
