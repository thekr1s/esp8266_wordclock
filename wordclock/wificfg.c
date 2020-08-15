/*
 * WiFi configuration via a simple web server.
 *
 * Copyright (C) 2016 OurAirQuality.org
 *
 * Licensed under the Apache License, Version 2.0, January 2004 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *      http://www.apache.org/licenses/
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS WITH THE SOFTWARE.
 *
 */

#include <string.h>
#include <ctype.h>

#include <espressif/esp_common.h>
#include <espressif/user_interface.h>
#include <esp/uart.h>
#include <sdk_internal.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <dhcpserver.h>

#include <lwip/api.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"

#include "wificfg.h"
#include "http_server.h"
#include "sysparam.h"

#include "ota_basic.h"
#include "settings.h"
#include "controller.h"
#include "esp_glue.h"

typedef enum {
    INVALID,
    CONNECTED,
    SOFT_AP,
} wifiState_t;

static char* _wifi_ap_ip_addr = "192.168.1.1";
TaskHandle_t _http_task_handle;
TaskHandle_t _dns_task_handle;

static void dns_task(void *pvParameters)
{
    ip_addr_t server_addr;
    server_addr.addr = ipaddr_addr(_wifi_ap_ip_addr);

    struct sockaddr_in serv_addr;
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(53);
    bind(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    const struct timeval timeout = { 2, 0 }; /* 2 second timeout */
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    const struct ifreq ifreq1 = { "en1" };
    setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifreq1, sizeof(ifreq1));
    
    for (;;) {
        char buffer[96];
        struct sockaddr src_addr;
        socklen_t src_addr_len = sizeof(src_addr);
        size_t count = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&src_addr, &src_addr_len);

        /* Drop messages that are too large to send a response in the buffer */
        if (count > 0 && count <= sizeof(buffer) - 16 && src_addr.sa_family == AF_INET) {
            size_t qname_len = strlen(buffer + 12) + 1;
            uint32_t reply_len = 2 + 10 + qname_len + 16 + 4;

            char *head = buffer + 2;
            *head++ = 0x80; // Flags
            *head++ = 0x00;
            *head++ = 0x00; // Q count
            *head++ = 0x01;
            *head++ = 0x00; // A count
            *head++ = 0x01;
            *head++ = 0x00; // Auth count
            *head++ = 0x00;
            *head++ = 0x00; // Add count
            *head++ = 0x00;
            head += qname_len;
            *head++ = 0x00; // Q type
            *head++ = 0x01;
            *head++ = 0x00; // Q class
            *head++ = 0x01;
            *head++ = 0xC0; // LBL offs
            *head++ = 0x0C;
            *head++ = 0x00; // Type
            *head++ = 0x01;
            *head++ = 0x00; // Class
            *head++ = 0x01;
            *head++ = 0x00; // TTL
            *head++ = 0x00;
            *head++ = 0x00;
            *head++ = 0x78;
            *head++ = 0x00; // RD len
            *head++ = 0x04;
            *head++ = ip4_addr1(&server_addr);
            *head++ = ip4_addr2(&server_addr);
            *head++ = ip4_addr3(&server_addr);
            *head++ = ip4_addr4(&server_addr);
            printf("DNS query, sending response\n");
            sendto(fd, buffer, reply_len, 0, &src_addr, src_addr_len);
        }
        uint32_t task_value = 0;
        if (xTaskNotifyWait(0, 1, &task_value, 0) == pdTRUE) {
            if (task_value)
                break;
        }
    }
    printf("Stopping DNS server");
    lwip_close(fd);
    vTaskDelete(NULL);
}

static void dns_start() {
    xTaskCreate(dns_task, "wifi_config DNS", 384, NULL, 2, &_dns_task_handle);
}

static void dns_stop() {
    if (!_dns_task_handle) {
        return;
    }
    xTaskNotify(_dns_task_handle, 1, eSetValueWithOverwrite);
}

static void wificfg_start_softAP() {
    sdk_wifi_set_opmode(STATIONAP_MODE);

    uint32_t chip_id = sdk_system_get_chip_id();
    struct sdk_softap_config ap_config = {
        .ssid_hidden = 0,
        .channel = 3,
        .authmode = AUTH_OPEN,
        .max_connection = 3,
        .beacon_interval = 100,
    };
    ap_config.ssid_len = snprintf((char *)ap_config.ssid, sizeof(ap_config.ssid),
        "woordklok%08x", chip_id);
    sdk_wifi_softap_set_config(&ap_config);
    printf("Start WiFi AccessPoint %s\n", ap_config.ssid);

    struct ip_info ap_ip;
    ap_ip.ip.addr = ipaddr_addr(_wifi_ap_ip_addr);
    ap_ip.netmask.addr = ipaddr_addr("255.255.255.0");
    IP4_ADDR(&ap_ip.gw, 0, 0, 0, 0);
    sdk_wifi_set_ip_info(SOFTAP_IF, &ap_ip);
    printf("Starting DHCP server\n");

    int8_t wifi_ap_dhcp_leases = 4;
    ip4_addr_t first_client_ip;
    first_client_ip.addr = ap_ip.ip.addr + htonl(1);

    dhcpserver_start(&first_client_ip, wifi_ap_dhcp_leases);
    dhcpserver_set_router(&ap_ip.ip);
    dhcpserver_set_dns(&ap_ip.ip);
    
    dns_start();
    wifi_scan_ap_start();
}

static void wificfg_stop_soft_AP() {
    dhcpserver_stop();
    dns_stop();
    wifi_scan_ap_stop();
    sdk_wifi_set_opmode(STATION_MODE);
}

static void wifi_monitor_task(void *pvParameters) {
    // Give the ESP some time to startup.
    for (int i = 0; i<5; i++) {
        if (sdk_wifi_station_get_connect_status() != STATION_CONNECTING) {
            printf("Station is done!\n");
            break;
        }
        printf("Station is busy connecting, wait...\n");
        Sleep(2000);
    }
    
    http_server_start(); //socket is reused for soft AP and normal mode
    
    wifiState_t state = INVALID;
    while (true) {
        if (sdk_wifi_station_get_connect_status() != STATION_GOT_IP) {
            if (state != SOFT_AP) { //only do something on a state change
                printf("Woordclock is unable to connect to Wifi\n");
                wificfg_start_softAP();
                state = SOFT_AP;
            }
        } else {
            if (state != CONNECTED) { 
                printf("Woordclock is now connected to the Wifi\n");
                wificfg_stop_soft_AP();
                state = CONNECTED;
            }
        }
        Sleep(10 * 1000);
    }
}

void wificfg_init()
{
    xTaskCreate(wifi_monitor_task, "WiFi monitor", 256, NULL, 2, NULL);
}
