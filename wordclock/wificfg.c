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

#define SLEEP_TIME                  (1000)
#define RECONNECTING_TIMEOUT_SEC    (30 * 60)

typedef enum {
    WIFI_STATE_IDLE,
    WIFI_STATE_SOFT_AP_STARTED,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_CONNECTION_LOST,
} EWifiState;

// NOTE this should match the esp_sta.h STATION enum
static char *station_status_strings[] =  {"idle", \
                                "connecting", \
                                "wrong password", \
                                "no ap found", \
                                "connect failed", \
                                "got IP address"};


static EWifiState _state = WIFI_STATE_IDLE;
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
            if (task_value) {
                break;
            }
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
    printf("Starting Soft accesspoint, dhcp server and dns server\r\n");

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
    _state = WIFI_STATE_SOFT_AP_STARTED;
}

static void wificfg_stop_soft_AP() {
    printf("Stopping Soft accesspoint, dhcp server and dns server\r\n");
    dhcpserver_stop();
    dns_stop();
    wifi_scan_ap_stop();
    
    sdk_wifi_set_opmode(STATION_MODE);
}

static void wifi_monitor_task(void *pvParameters) 
{
    uint32_t timeout = 0;

    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_auto_connect(TRUE);
    Sleep(5 * 1000); // Let the esp connection settle for 5 seconds

    while (true) {
        Sleep(SLEEP_TIME);
        uint8_t status = sdk_wifi_station_get_connect_status();
        switch (status) {
            case STATION_IDLE:
            case STATION_CONNECTING:
            case STATION_WRONG_PASSWORD:
            case STATION_NO_AP_FOUND:
            case STATION_CONNECT_FAIL:
                printf("WiFi status: %s.\r\n", station_status_strings[status]);
                switch (_state) {
                    case WIFI_STATE_IDLE:
                        wificfg_start_softAP();
                    break;
                    case WIFI_STATE_SOFT_AP_STARTED:
                        // Do nothing
                    break;
                    case WIFI_STATE_CONNECTED:
                        timeout = 0;
                        _state = WIFI_STATE_CONNECTION_LOST;
                    case WIFI_STATE_CONNECTION_LOST:
                        timeout ++;
                        printf("Lost connection for %d seconds\r\n", timeout);
                        if(timeout > RECONNECTING_TIMEOUT_SEC) {
                            printf("Reconnect timout, Reboot!\r\n");
                            SleepNI(100);
                            sdk_system_restart();
                        }
                    break;
                }
            break;
            case STATION_GOT_IP:
                if (_state == WIFI_STATE_IDLE || _state == WIFI_STATE_SOFT_AP_STARTED) {
                    printf("WiFi: connected.\r\n");
                    wificfg_stop_soft_AP();
                }
                _state = WIFI_STATE_CONNECTED;   
            break;
        }
    }
}

void wificfg_init()
{
    xTaskCreate(wifi_monitor_task, "WiFi monitor", 256, NULL, 3, NULL);
}
