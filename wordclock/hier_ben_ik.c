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

#include "espressif/esp_common.h"

#include "esp_glue.h"
#include "settings.h"

uint32_t _dist = 10000;
uint32_t _age = 10000;


void HbiGetDistAndAge(uint32_t* pDist, uint32_t* pAge) {
	// TODO
	*pDist = _dist;
	*pAge = _age;
}

static void ParseResponse(char* pResp){
//	int i;
	const char* ageTag = "age:";
	const char* distTag = "dist:";
	char* p;
	int t, t2;

	p = strstr(pResp, ageTag);
	if (p == NULL){
	    return; //return else this result in hard fault
	}
	if (sscanf(p, "age:%d", &t) == 1) {
		_age = t;
		printf("age: %d\n", t);
	}
	p = strstr(pResp, distTag);
	if (sscanf(p, "dist:%d.%3d", &t, &t2) == 2) {
		_dist = t * 1000 + t2;
		printf("dist: %d\n", _dist);
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
			printf("DNS lookup failed err=%d res=%p\r\n", err, res);
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
	sprintf(tempRequest, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nAccept: */*\r\n\r\n", g_settings.hierbenikRequest, g_settings.hierbenikUrl);
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
	do {
		r = lwip_read(s, &recv_buf[i], sizeof(recv_buf) - i - 1);
		i += r;
	} while (r > 0);
//		printf("%s\n---\n", recv_buf);
	ParseResponse(recv_buf);

	if (r != 0)
		failures++;
	else
		successes++;
	closesocket(s);
	printf("%s %d leave, \n%s\n", __FUNCTION__, __LINE__, recv_buf);

}

void HbiTask(void *pvParameters){

	while (sdk_wifi_station_get_connect_status() != STATION_GOT_IP) {
		printf("%s: Wait for connect\n", __FUNCTION__);
		SleepNI(5000);
	}
	while (true) {
		GetHierBenIk();
		if (_age < 300) {
			SleepNI(10000);
		} else {
			SleepNI(5 * 60000);
		}

	}
}


void HbiInit() {
    xTaskCreate(HbiTask, "HierBenIk task", 1024, NULL, 1, NULL);
}
