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

#include "http_server.h"
#include "sysparam.h"
#include "ota_basic.h"
#include "settings.h"
#include "controller.h"
#include "esp_glue.h"
#include "hier_ben_ik.h"
#include "ldr.h"
#include "si4703.h"

#define MAX_SSID_LEN 20 
#define MAX_SSID_COUNT 25

static char _ad[] = "robert@wssns.nl";

static char _ssidList[MAX_SSID_COUNT][MAX_SSID_LEN+1]; // + zero termination
static int8_t _ssidRssi[MAX_SSID_COUNT];
static volatile int _ssidCount = 0;
static const char * const auth_modes [] = {
    [AUTH_OPEN]         = "Open",
    [AUTH_WEP]          = "WEP",
    [AUTH_WPA_PSK]      = "WPA/PSK",
    [AUTH_WPA2_PSK]     = "WPA2/PSK",
    [AUTH_WPA_WPA2_PSK] = "WPA/WPA2/PSK"
};

SemaphoreHandle_t wifi_networks_mutex;
TaskHandle_t _http_server_task_handle;
TaskHandle_t _wifi_scan_ap_task_handle = NULL;
static void scan_done_cb(void *arg, sdk_scan_status_t status)
{
    if (status != SCAN_OK) {
        printf("Error: WiFi scan failed\n");
        return;
    }

    struct sdk_bss_info *bss = (struct sdk_bss_info *)arg;
    // first one is invalid
    bss = bss->next.stqe_next;

    xSemaphoreTake(wifi_networks_mutex, portMAX_DELAY);

    _ssidCount = 0;
    while (NULL != bss)
    {
        strncpy(_ssidList[_ssidCount], (char*)bss->ssid, MAX_SSID_LEN);
        _ssidList[_ssidCount][MAX_SSID_LEN] = 0; // zero terminate to be sure
        _ssidRssi[_ssidCount] = bss->rssi;

        // printf("%32s (" MACSTR ") RSSI: %02d, security: %s\n", _ssidList[_ssidCount],
        //    MAC2STR(bss->bssid), bss->rssi, auth_modes[bss->authmode]);
        bss = bss->next.stqe_next;
        _ssidCount++;
    }
    // bubblesort the list
    bool done=false;
    char tmp[MAX_SSID_LEN];
    while (!done) {
        done = true;
        for (int i = 0; i < _ssidCount - 1; i++) {
            if (_ssidRssi[i] < _ssidRssi[i+1]) {
                // swap them
                strncpy(tmp, _ssidList[i], MAX_SSID_LEN);
                strncpy(_ssidList[i], _ssidList[i + 1], MAX_SSID_LEN);
                strncpy(_ssidList[i + 1], tmp, MAX_SSID_LEN);
                int8_t t = _ssidRssi[i];
                _ssidRssi[i] = _ssidRssi[i+1];
                _ssidRssi[i+1] = t;
                done = false;
            }
        }
    }
    xSemaphoreGive(wifi_networks_mutex);
    // printf("Updated WiFi network names\n");
}

/*
 * Read a line terminated by "\r\n" or "\n" to be robust. Used to read the http
 * status line and headers. On success returns the number of characters read,
 * which might be more that the available buffer size 'len'. Excess characters
 * in a line are discarded as a protection against excessively long lines. On
 * failure -1 is returned. The character case is lowered to give a canonical
 * case for easier comparision. The buffer is null terminated on success, even
 * if truncated.
 */
static int read_crlf_line(int s, char *buf, size_t len)
{
    size_t num = 0;

    do {
        char c;
        int r = lwip_read(s, &c, 1);

        /* Expecting a known terminator so fail on EOF. */
        if (r <= 0)
            return -1;

        if (c == '\n')
            break;

        /* Remove a trailing '\r', and many unexpected characters. */
        if (c < 0x20 || c > 0x7e)
            continue;

        if (num < len)
            buf[num] = tolower((unsigned char)c);

        num++;
    } while(1);

    /* Null terminate. */
    buf[num >= len ? len - 1 : num] = 0;

    return num;
}

int wificfg_form_name_value(int s, bool *valp, size_t *rem, char *buf, size_t len)
{
    size_t num = 0;

    do {
        if (*rem == 0)
            break;

        char c;
        int r = lwip_read(s, &c, 1);

        /* Expecting a known number of characters so fail on EOF. */
        if (r <= 0) return -1;

        (*rem)--;

        if (valp && c == '=') {
            *valp = true;
            break;
        }

        if (c == '&') {
            if (valp)
                *valp = false;
            break;
        }

        if (num < len)
            buf[num] = c;

        num++;
    } while(1);

    /* Null terminate. */
    buf[num >= len ? len - 1 : num] = 0;

    return num;
}

void wificfg_form_url_decode(char *string)
{
    char *src = string;
    char *src_end = string + strlen(string);
    char *dst = string;

    while (src < src_end) {
        char c = *src++;
        if (c == '+') {
            c = ' ';
        } else if (c == '%' && src < src_end - 1) {
            unsigned char c1 = src[0];
            unsigned char c2 = src[1];
            if (isxdigit(c1) && isxdigit(c2)) {
                c1 = tolower(c1);
                int d1 = (c1 >= 'a' && c1 <= 'z') ? c1 - 'a' + 10 : c1 - '0';
                c2 = tolower(c2);
                int d2 = (c2 >= 'a' && c2 <= 'z') ? c2 - 'a' + 10 : c2 - '0';
                *dst++ = (d1 << 4) + d2;
                src += 2;
                continue;
            }
        }
        *dst++ = c;
    }

    *dst = 0;
}

/* HTML escaping. */
void wificfg_html_escape(char *string, char *buf, size_t len)
{
    size_t i;
    size_t out = 0;

    for (i = 0, out = 0; out < len - 1; ) {
        char c = string[i++];
        if (!c)
            break;

        if (c == '&') {
            if (out >= len - 5)
                break;
            buf[out] = '&';
            buf[out + 1] = 'a';
            buf[out + 2] = 'm';
            buf[out + 3] = 'p';
            buf[out + 4] = ';';
            out += 5;
            continue;
        }
        if (c == '"') {
            if (out >= len - 6)
                break;
            buf[out] = '&';
            buf[out + 1] = 'q';
            buf[out + 2] = 'u';
            buf[out + 3] = 'o';
            buf[out + 4] = 't';
            buf[out + 5] = ';';
            out += 6;
            continue;
        }
        if (c == '<') {
            if (out >= len - 4)
                break;
            buf[out] = '&';
            buf[out + 1] = 'l';
            buf[out + 2] = 't';
            buf[out + 3] = ';';
            out += 4;
            continue;
        }
        if (c == '>') {
            if (out >= len - 4)
                break;
            buf[out] = '&';
            buf[out + 1] = 'g';
            buf[out + 2] = 't';
            buf[out + 3] = ';';
            out += 4;
            continue;
        }

        buf[out++] = c;
    }

    buf[out] = 0;
}

/* Various keywords are interned as they are read. */

static const struct {
    const char *str;
    wificfg_method method;
} method_table[] = {
    {"get", HTTP_METHOD_GET},
    {"post", HTTP_METHOD_POST},
    {"head", HTTP_METHOD_HEAD}
};

static wificfg_method intern_http_method(char *str)
{
    int i;
    for (i = 0;  i < sizeof(method_table) / sizeof(method_table[0]); i++) {
        if (!strcmp(str, method_table[i].str))
            return method_table[i].method;
    }
    return HTTP_METHOD_OTHER;
}

/*
 * The web server recognizes only these header names. Other headers are ignored.
 */
typedef enum {
    HTTP_HEADER_HOST,
    HTTP_HEADER_CONTENT_LENGTH,
    HTTP_HEADER_CONTENT_TYPE,
    HTTP_HEADER_OTHER
} http_header;

static const struct {
    const char *str;
    http_header name;
} http_header_table[] = {
    {"host", HTTP_HEADER_HOST},
    {"content-length", HTTP_HEADER_CONTENT_LENGTH},
    {"content-type", HTTP_HEADER_CONTENT_TYPE}
};

static http_header intern_http_header(char *str)
{
    int i;
    for (i = 0;  i < sizeof(http_header_table) / sizeof(http_header_table[0]); i++) {
        if (!strcmp(str, http_header_table[i].str))
            return http_header_table[i].name;
    }
    return HTTP_HEADER_OTHER;
}


static const struct {
    const char *str;
    wificfg_content_type type;
} content_type_table[] = {
    {"application/x-www-form-urlencoded", HTTP_CONTENT_TYPE_WWW_FORM_URLENCODED}
};

static wificfg_content_type intern_http_content_type(char *str)
{
    int i;
    for (i = 0;  i < sizeof(content_type_table) / sizeof(content_type_table[0]); i++) {
        if (!strcmp(str, content_type_table[i].str))
            return content_type_table[i].type;
    }
    return HTTP_CONTENT_TYPE_OTHER;
}

static char *skip_whitespace(char *string)
{
    while (isspace((unsigned char)*string)) string++;
    return string;
}

static char *skip_to_whitespace(char *string)
{
    do {
        unsigned char c = *string;
        if (!c || isspace(c))
            break;
        string++;
    } while (1);

    return string;
}

int wificfg_write_string(int s, const char *str)
{
    int res = lwip_write(s, str, strlen(str));
    return res;
}

typedef enum {
    FORM_NAME_STA_SSID,
    FORM_NAME_STA_PASSWORD,
    FORM_NAME_STA_SSID_DROPDOWN,
    FORM_NAME_STA_COMMAND,
    FORM_NAME_NONE
} form_name;

static const struct {
    const char *str;
    form_name name;
} form_name_table[] = {
    {"sta_ssid", FORM_NAME_STA_SSID},
    {"sta_password", FORM_NAME_STA_PASSWORD},
    {"select_ap", FORM_NAME_STA_SSID_DROPDOWN},
    {"sta_command", FORM_NAME_STA_COMMAND},
};

static form_name intern_form_name(char *str)
{
     int i;
     for (i = 0;  i < sizeof(form_name_table) / sizeof(form_name_table[0]); i++) {
         if (!strcmp(str, form_name_table[i].str))
             return form_name_table[i].name;
     }
     return FORM_NAME_NONE;
}


static const char http_favicon[] =
#include "content/favicon.ico"
;

static void handle_favicon(int s, wificfg_method method,
                           uint32_t content_length,
                           wificfg_content_type content_type,
                           char *buf, size_t len)
{
    wificfg_write_string(s, http_favicon);

}

// .value-lg{font-size:24px}.label-extra{display:block;font-style:italic;font-size:13px}
// devo: "Cache-Control: no-store\r\n"
static const char http_style[] =
#include "content/style.css"
;


static void handle_style(int s, wificfg_method method,
                         uint32_t content_length,
                         wificfg_content_type content_type,
                         char *buf, size_t len)
{
    wificfg_write_string(s, http_style);
}

static const char http_iro[] =
#include "content/iro.js"
;

static void handle_iro(int s, wificfg_method method,
                           uint32_t content_length,
                           wificfg_content_type content_type,
                           char *buf, size_t len)
{
    wificfg_write_string(s, http_iro);
}

static const char http_colorPicker[] =
#include "content/colorpicker.js"
;

static void handle_colorPicker(int s, wificfg_method method,
                           uint32_t content_length,
                           wificfg_content_type content_type,
                           char *buf, size_t len)
{
    wificfg_write_string(s, http_colorPicker);
}


static const char http_script[] =
#include "content/script.js"
;

static void handle_script(int s, wificfg_method method,
                           uint32_t content_length,
                           wificfg_content_type content_type,
                           char *buf, size_t len)
{
    wificfg_write_string(s, http_script);
}


static const char http_success_header[] = "HTTP/1.0 200 \r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "Cache-Control: no-store\r\n"
    "\r\n";

static const char http_redirect_header_softap [] = "HTTP/1.1 302 \r\n"
    "Location: http://192.168.1.1/wificfg/sta.html\r\n"
    "Content-Length: 0\r\n"
    "Connection: close\r\n"
    "\r\n";

static const char http_redirect_header_sta[] = "HTTP/1.0 302 \r\n"
    "Location: /wificfg/sta.html\r\n"
    "\r\n";

static const char http_redirect_header_clockcfg[] = "HTTP/1.0 302 \r\n"
    "Location: /wificfg/clockcfg.html\r\n"
    "\r\n";

static const char http_redirect_header_hwcfg[] = "HTTP/1.0 302 \r\n"
    "Location: /wificfg/hwcfg.html\r\n"
    "\r\n";

static const char http_redirect_header_debug[] = "HTTP/1.0 302 \r\n"
    "Location: /wificfg/debug.html\r\n"
    "\r\n";

static const char http_redirect_header_controller[] = "HTTP/1.0 302 \r\n"
    "Location: /wificfg/controller.html\r\n"
    "\r\n";

static const char http_redirect_header_delayed[] = "<html> <head>"
        "<meta http-equiv=\"refresh\" content=\"10;url=/wificfg/clockcfg.html\" />"
    "</head> <body> <h1>Resetting, refresh manually when clock is up...</h1> </body></html>";

static void handle_wificfg_redirect(int s, wificfg_method method,
                                    uint32_t content_length,
                                    wificfg_content_type content_type,
                                    char *buf, size_t len)
{
    wificfg_write_string(s, http_redirect_header_clockcfg);
}

static const char *http_wifi_station_content[] = {
#include "content/wificfg/sta.html"
};

static void handle_wifi_station(int s, wificfg_method method,
                                uint32_t content_length,
                                wificfg_content_type content_type,
                                char *buf, size_t len)
{
    wifi_scan_ap_start(); //In case the user wands to reconfigure the wifi settings the scan task needs to be started.
    if (wificfg_write_string(s, http_success_header) < 0) return;

    if (method != HTTP_METHOD_HEAD) {
        if (wificfg_write_string(s, http_wifi_station_content[0]) < 0) return;
        wificfg_write_string(s, "<option value=\"100\">Select...</option>");
        static char line[100];
        xSemaphoreTake(wifi_networks_mutex, portMAX_DELAY);
        for (int i = 0; i < _ssidCount; i++) {
            snprintf(line, sizeof(line), "<option value=\"%s\">%s</option>", _ssidList[i], _ssidList[i]);
            if (wificfg_write_string(s, line) < 0) {
                break;
            }
        }
        xSemaphoreGive(wifi_networks_mutex);
        if (wificfg_write_string(s, http_wifi_station_content[1]) < 0) return;
    }
    closesocket(s);
}

static void handle_wifi_station_post(int s, wificfg_method method,
                                     uint32_t content_length,
                                     wificfg_content_type content_type,
                                     char *buf, size_t len)
{
	static char ssid[50] = "";
	static char password[50] = "";

    if (content_type != HTTP_CONTENT_TYPE_WWW_FORM_URLENCODED) {
        wificfg_write_string(s, "HTTP/1.0 400 \r\nContent-Type: text/html\r\n\r\n");
        return;
    }

    printf("BUF: %s\n", buf);
    size_t rem = content_length;
    bool valp = false;

    while (rem > 0) {
        int r = wificfg_form_name_value(s, &valp, &rem, buf, len);

        if (r < 0) {
            break;
        }

        wificfg_form_url_decode(buf);

        form_name name = intern_form_name(buf);

        if (valp) {
            int r = wificfg_form_name_value(s, NULL, &rem, buf, len);
            if (r < 0) {
                break;
            }

            wificfg_form_url_decode(buf);
            printf("handle_wifi_station_post %d %s\n", name, buf);
            switch (name) {
            case FORM_NAME_STA_SSID_DROPDOWN:
                strncpy(ssid, buf, sizeof(ssid) - 1);
                printf("Selected ssid: %s\r\n", ssid);
                break;
            case FORM_NAME_STA_SSID:
                sysparam_set_string("wifi_sta_ssid", buf);
                if (buf[0]!='\0') {
                    bzero(ssid, sizeof(ssid));
                    strncpy(ssid, buf, sizeof(ssid) - 1);
                }
                break;
            case FORM_NAME_STA_PASSWORD:
                sysparam_set_string("wifi_sta_password", buf);
                bzero(password, sizeof(password));
                strncpy(password, buf, sizeof(password) - 1);
                break;
            case FORM_NAME_STA_COMMAND:
                printf("Buf: %s, %s %s\r\n", buf, ssid, password);
                if (strcmp(buf, "Save") == 0) {
                    if (ssid[0] != '\0' && password[0] != '\0') {
                        wificfg_write_string(s, "Rebooting with new WIFI settings\r\n");
                        closesocket(s);
                        struct sdk_station_config config = {"", "", 0, {0}};
                        strncpy((char*)config.ssid, ssid, sizeof(config.ssid));
                        strncpy((char*)config.password, password, sizeof(config.password));
                        if (!sdk_wifi_station_set_config(&config)) {
                            printf("ERROR sdk_wifi_station_set_config\n");
                        }
                        SleepNI(1000);
                        sdk_system_restart();
                    } else if (strcmp(buf, "Refresh") == 0) {
                        printf("Refresh page !!!\n");
                    } else {
                        closesocket(s);
                        return;
                    }
                }
                break;
            default:
                break;
            }
        }
    }

    wificfg_write_string(s, http_redirect_header_sta);
}


static const char *http_clock_cfg_content[] = {
#include "content/wificfg/clockcfg.html"
};

static void handle_clock_cfg(int s, wificfg_method method,
                                uint32_t content_length,
                                wificfg_content_type content_type,
                                char *buf, size_t len)
{
	int idx = 0;
	char tmpStr[20];
	printf("%s %d\n", __FUNCTION__, __LINE__);
    if (wificfg_write_string(s, http_success_header) < 0) return;

    if (method != HTTP_METHOD_HEAD) {
    	if (wificfg_write_string(s, http_clock_cfg_content[idx]) < 0) return;
        
    	// Color
        if (wificfg_write_string(s, http_clock_cfg_content[++idx]) < 0) return;
        uint32_t hexColor = (g_settings.color.r << 16) | (g_settings.color.g << 8) | (g_settings.color.b << 0);
        bzero(tmpStr, sizeof(tmpStr));
        snprintf(tmpStr, sizeof(tmpStr)-1, "#%06x", hexColor);
        if (wificfg_write_string(s, tmpStr) < 0) return;
        
        // Text Effect
        for (int i = 0; i < TEXTEFFECT_MAX; i++) {
        	wificfg_write_string(s, http_clock_cfg_content[++idx]);
        	if (g_settings.textEffect == i) wificfg_write_string(s, "selected");
        }

        // Brightness
        if (wificfg_write_string(s, http_clock_cfg_content[++idx]) < 0) return;
        bzero(tmpStr, sizeof(tmpStr));
        snprintf(tmpStr, sizeof(tmpStr)-1, "%d", g_settings.brightnessOffset);
        if (wificfg_write_string(s, tmpStr) < 0) return;

        // Background Color
        if (wificfg_write_string(s, http_clock_cfg_content[++idx]) < 0) return;
        hexColor = (g_settings.bgColor.r << 16) | (g_settings.bgColor.g << 8) | (g_settings.bgColor.b << 0);
        bzero(tmpStr, sizeof(tmpStr));
        snprintf(tmpStr, sizeof(tmpStr)-1, "#%06x", hexColor);
        if (wificfg_write_string(s, tmpStr) < 0) return;

        // Animations
        for (int i = 0; i < ANIMATION_MAX; i++) {
        	wificfg_write_string(s, http_clock_cfg_content[++idx]);
        	if (g_settings.animation == i) wificfg_write_string(s, "selected");
        }

        // Message text
        if (wificfg_write_string(s, http_clock_cfg_content[++idx]) < 0) return;
        wificfg_write_string(s,AnimationGetMessageText());

        if (wificfg_write_string(s, http_clock_cfg_content[++idx]) < 0) return;
    }
}

static void handle_clock_cfg_post(int s, wificfg_method method,
                                     uint32_t content_length,
                                     wificfg_content_type content_type,
                                     char *buf, size_t len)
{

    if (content_type != HTTP_CONTENT_TYPE_WWW_FORM_URLENCODED) {
        wificfg_write_string(s, "HTTP/1.0 400 \r\nContent-Type: text/html\r\n\r\n");
        return;
    }

    printf("BUF: %s\n", buf);
    size_t rem = content_length;
    bool valp = false;

    while (rem > 0) {
        int r = wificfg_form_name_value(s, &valp, &rem, buf, len);

        if (r < 0) {
            break;
        }

        wificfg_form_url_decode(buf);

        char name[30];
        bzero(name, sizeof(name));
        strncpy(name, buf, sizeof(name) - 1);

        if (valp) {
            int r = wificfg_form_name_value(s, NULL, &rem, buf, len);
            if (r < 0) {
                break;
            }

            wificfg_form_url_decode(buf);
            printf("%s %s %s\n", __FUNCTION__, name, buf);
            if (strcmp(name, "cl_color") == 0) {
                uint32_t hexColor = 0;
                if (sscanf(buf, "#%06x", &hexColor) == 1 && hexColor <= 0xffffff) {
                    g_settings.color.r = hexColor >> 16 & 0xff; 
                    g_settings.color.g = hexColor >> 8  & 0xff;
                    g_settings.color.b = hexColor >> 0  & 0xff;
                } else {
                    printf("Received invalid Color value\r\n");
                }
            } else if (strcmp(name, "cl_textEffect") == 0) {
            	g_settings.textEffect = atoi(buf);
            } else if (strcmp(name, "cl_brightness") == 0) {
            	g_settings.brightnessOffset = atoi(buf);
            } else if (strcmp(name, "cl_bgcolor") == 0) {
                uint32_t hexColor = 0;
                if (sscanf(buf, "#%06x", &hexColor) == 1 && hexColor <= 0xffffff) {
                    g_settings.bgColor.r = hexColor >> 16 & 0xff; 
                    g_settings.bgColor.g = hexColor >> 8  & 0xff;
                    g_settings.bgColor.b = hexColor >> 0  & 0xff;
                } else {
                    printf("Received invalid background Color value\r\n");
                }
            } else if (strcmp(name, "cl_animations") == 0) {
            	g_settings.animation = atoi(buf);
            } else if (strcmp(name, "cl_message") == 0) {
            	AnimationSetMessageText(buf);
            }
        }
    }
    if (g_settings.textEffect == TEXTEFFECT_RANDOM) {
        SettingRandomColors();
    }

    SettingsScheduleStore();
    wificfg_write_string(s, http_redirect_header_clockcfg);
    SetInterrupted(true);
}

static const char *http_hw_cfg_content[] = {
#include "content/wificfg/hwcfg.html"
};

static void handle_hw_cfg(int s, wificfg_method method,
                                uint32_t content_length,
                                wificfg_content_type content_type,
                                char *buf, size_t len)
{
	int idx = 0;
    char tempStr[20];
	printf("%s %d\n", __FUNCTION__, __LINE__);
    if (wificfg_write_string(s, http_success_header) < 0) return;
	printf("%s %d\n", __FUNCTION__, __LINE__);

    if (method != HTTP_METHOD_HEAD) {
    	if (wificfg_write_string(s, http_hw_cfg_content[idx]) < 0) return;
    	// Hardware Version
		for (int i = 0; i < NR_OF_HARDWARE_TYPES; i++) {
			if (wificfg_write_string(s, http_hw_cfg_content[++idx]) < 0) return;
			if (g_hw_settings.hardwareType == i) wificfg_write_string(s, "selected");
		}

        // Pixel Types
		for (int i = 0; i < NR_OF_PIXEL_TYPES; i++) {
			if (wificfg_write_string(s, http_hw_cfg_content[++idx]) < 0) return;
			if (g_hw_settings.pixelType == i) wificfg_write_string(s, "selected");
		}
        // Perfect Imperfections
        wificfg_write_string(s, http_hw_cfg_content[++idx]);
        if (g_settings.perfectImperfections == 1) wificfg_write_string(s, "checked");

        // Zomer/Winter tijd correctie
        wificfg_write_string(s, http_hw_cfg_content[++idx]);
        if (g_settings.correctDST == 1) wificfg_write_string(s, "checked");

        // Timezone offset (UTC)
        if (wificfg_write_string(s, http_hw_cfg_content[++idx]) < 0) return;
        bzero(tempStr, sizeof(tempStr));
        snprintf(tempStr, sizeof(tempStr)-1, "%02d:%02d", g_settings.timeZoneOffsetMinuts/60, abs(g_settings.timeZoneOffsetMinuts%60));
        if (wificfg_write_string(s, tempStr) < 0) return;
        
        // HierBenIk url
        if (wificfg_write_string(s, http_hw_cfg_content[++idx]) < 0) return;
        wificfg_write_string(s, g_settings.hierbenikUrl);
        // HierBenIk port
        if (wificfg_write_string(s, http_hw_cfg_content[++idx]) < 0) return;
        wificfg_write_string(s, g_settings.hierbenikPort);
        // HierBenIk request
        if (wificfg_write_string(s, http_hw_cfg_content[++idx]) < 0) return;
        wificfg_write_string(s, g_settings.hierbenikRequest);

        // HierBenIk home lat
        if (wificfg_write_string(s, http_hw_cfg_content[++idx]) < 0) return;
        snprintf(tempStr, sizeof(tempStr), "%f", g_settings.hierbenikHomeLat);
        wificfg_write_string(s, tempStr);
        // HierBenIk home lon
        if (wificfg_write_string(s, http_hw_cfg_content[++idx]) < 0) return;
        snprintf(tempStr, sizeof(tempStr), "%f", g_settings.hierbenikHomeLon);
        wificfg_write_string(s, tempStr);

        // FW update url
        wificfg_write_string(s, http_hw_cfg_content[++idx]);
        if (wificfg_write_string(s, g_settings.otaFwUrl) < 0) return;
        // FW update port
        wificfg_write_string(s, http_hw_cfg_content[++idx]);
        if (wificfg_write_string(s, g_settings.otaFwPort) < 0) return;
        // FW update Type
        for (int i = 0; i < 2; i++) {
            if (wificfg_write_string(s, http_hw_cfg_content[++idx]) < 0) return;
            if (g_settings.otaFwType == i) wificfg_write_string(s, "selected");
        }

        if (wificfg_write_string(s, http_hw_cfg_content[++idx]) < 0) return;

        wificfg_write_string(s, _ad);
        wificfg_write_string(s, "<br/>build: ");
        wificfg_write_string(s, buildDate);
        
        wificfg_write_string(s, "<br/>rev. : ");
        wificfg_write_string(s, version);
        printf("git:%s\n", version);
    }
}

static void handle_hw_cfg_post(int s, wificfg_method method,
                                     uint32_t content_length,
                                     wificfg_content_type content_type,
                                     char *buf, size_t len)
{
    if (content_type != HTTP_CONTENT_TYPE_WWW_FORM_URLENCODED) {
        wificfg_write_string(s, "HTTP/1.0 400 \r\nContent-Type: text/html\r\n\r\n");
        return;
    }

    printf("BUF: %s\n", buf);
    char name[30];
    int tmp1, tmp2;
    size_t rem = content_length;
    bool valp = false;

    //Checkbox don't send anything when unchecked, so first set the value to unset.
    g_settings.perfectImperfections = 0;
    g_settings.correctDST = 0;
    while (rem > 0) {
        int r = wificfg_form_name_value(s, &valp, &rem, buf, len);

        if (r < 0) {
            break;
        }

        wificfg_form_url_decode(buf);
        bzero(name, sizeof(name));
        strncpy(name, buf, sizeof(name) - 1);

        if (valp) {
            int r = wificfg_form_name_value(s, NULL, &rem, buf, len);
            if (r < 0) {
                break;
            }

            wificfg_form_url_decode(buf);
            printf("%s %s %s\n", __FUNCTION__, name, buf);
            if (strcmp(name, "hw_hardwaretype") == 0) {
                g_hw_settings.hardwareType = atoi(buf);
            } else if (strcmp(name, "hw_pixeltype") == 0) {
                g_hw_settings.pixelType = atoi(buf);
            } else if (strcmp(name, "hw_imperfections") == 0) {
                if (strstr(buf, "CheckOn") != NULL) {
                    g_settings.perfectImperfections = 1;
                }
            } else if (strcmp(name, "hw_correctdst") == 0) {
                if (strstr(buf, "CheckOn") != NULL) {
                    g_settings.correctDST = 1;
                }
            } else if (strcmp(name, "hw_timezone") == 0) {
                if (sscanf(buf, "%d:%d", &tmp1, &tmp2) == 2) {
                    if (tmp1 > 0) {
                        g_settings.timeZoneOffsetMinuts = (tmp1 * 60) + tmp2;
                    } else {
                        g_settings.timeZoneOffsetMinuts = (tmp1 * 60) - tmp2;
                    }
                }
            } else if (strcmp(name, "hw_hierbenik_url") == 0) {
                bzero(g_settings.hierbenikUrl, sizeof(g_settings.hierbenikUrl));
                strncpy(g_settings.hierbenikUrl, buf, sizeof(g_settings.hierbenikUrl) - 1);
            } else if (strcmp(name, "hw_hierbenik_port") == 0) {
                bzero(g_settings.hierbenikPort, sizeof(g_settings.hierbenikPort));
                strncpy(g_settings.hierbenikPort, buf, sizeof(g_settings.hierbenikPort) - 1);
            } else if (strcmp(name, "hw_hierbenik_req") == 0) {
                bzero(g_settings.hierbenikRequest, sizeof(g_settings.hierbenikRequest));
                strncpy(g_settings.hierbenikRequest, buf, sizeof(g_settings.hierbenikRequest) - 1);
            } else if (strcmp(name, "home_lat") == 0) {
                sscanf(buf, "%f", &g_settings.hierbenikHomeLat);
            } else if (strcmp(name, "home_lon") == 0) {
                sscanf(buf, "%f", &g_settings.hierbenikHomeLon);
            } else if (strcmp(name, "hw_otafw_url") == 0) {
                bzero(g_settings.otaFwUrl, sizeof(g_settings.otaFwUrl));
                strncpy(g_settings.otaFwUrl, buf, sizeof(g_settings.otaFwUrl) - 1);
            } else if (strcmp(name, "hw_otafw_port") == 0) {
                bzero(g_settings.otaFwPort, sizeof(g_settings.otaFwPort));
                strncpy(g_settings.otaFwPort, buf, sizeof(g_settings.otaFwPort) - 1);
            } else if (strcmp(name, "image_Type") == 0) {
                g_settings.otaFwType = atoi(buf);
            } else if (strcmp(name, "cl_command") == 0) {
                printf("cl_command: %s\n", buf);
            	if (strcmp(buf, "SetHome") == 0){
                    HbiGetLatLon(&g_settings.hierbenikHomeLat, &g_settings.hierbenikHomeLon);
                    printf("%f %f\n", g_settings.hierbenikHomeLat, g_settings.hierbenikHomeLon);
            	} else if (strcmp(buf, "Save") == 0){
                    SettingsWrite();
            	} else if (strcmp(buf, "OtaUpdate") == 0){
            		OtaUpdateInit();
            		wificfg_write_string(s, "OTA UPDATING");
            		SetInterrupted(true);
            		return;
            	} else if (strcmp(buf, "Reboot") == 0){
                    wificfg_write_string(s, "Rebooting\r\n");
                    closesocket(s);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    sdk_system_restart();
            	} else if (strcmp(buf, "FactoryReset") == 0){
            	    wificfg_write_string(s, http_redirect_header_delayed);
            		SettingsClockReset();
            	}
           		
            }
        }
    }

    wificfg_write_string(s, http_redirect_header_hwcfg);

}

static const char *http_controller_content[] = {
#include "content/wificfg/controller.html"
};

static void handle_controller(int s, wificfg_method method,
                                uint32_t content_length,
                                wificfg_content_type content_type,
                                char *buf, size_t len)
{
	int idx = 0;
	printf("%s %d\n", __FUNCTION__, __LINE__);
    if (wificfg_write_string(s, http_success_header) < 0) return;
	printf("%s %d\n", __FUNCTION__, __LINE__);

    if (method != HTTP_METHOD_HEAD) {
    	if (wificfg_write_string(s, http_controller_content[idx]) < 0) return;
    }
}

static void handle_controller_post(int s, wificfg_method method,
                                     uint32_t content_length,
                                     wificfg_content_type content_type,
                                     char *buf, size_t len)
{
    if (content_type != HTTP_CONTENT_TYPE_WWW_FORM_URLENCODED) {
        wificfg_write_string(s, "HTTP/1.0 400 \r\nContent-Type: text/html\r\n\r\n");
        return;
    }

    printf("BUF: %s\n", buf);
    size_t rem = content_length;
    bool valp = false;

    while (rem > 0) {
        int r = wificfg_form_name_value(s, &valp, &rem, buf, len);

        if (r < 0) {
            break;
        }

        wificfg_form_url_decode(buf);

        char name[30];
        bzero(name, sizeof(name));
        strncpy(name, buf, sizeof(name) - 1);

        if (valp) {
            int r = wificfg_form_name_value(s, NULL, &rem, buf, len);
            if (r < 0) {
                break;
            }

            wificfg_form_url_decode(buf);
            printf("%s %s %s\n", __FUNCTION__, name, buf);
            if (strcmp(name, "cl_command") == 0) {
            	if (strcmp(buf, "Up") == 0){
                    ControllerSet(CONTROLLER_UP);
            		SetInterrupted(true);
            	} else if (strcmp(buf, "UpLeft") == 0){
                    ControllerSet(CONTROLLER_UP_LEFT);
            		SetInterrupted(true);
            	} else if (strcmp(buf, "UpRight") == 0){
                    ControllerSet(CONTROLLER_UP_RIGHT);
            		SetInterrupted(true);
            	} else if (strcmp(buf, "Left") == 0){
                    ControllerSet(CONTROLLER_LEFT);
            		SetInterrupted(true);
            	} else if (strcmp(buf, "Center") == 0){
                    ControllerSet(CONTROLLER_CENTER);
            		SetInterrupted(true);
            	} else if (strcmp(buf, "Right") == 0){
                    ControllerSet(CONTROLLER_RIGHT);
            		SetInterrupted(true);
            	} else if (strcmp(buf, "Down") == 0){
                    ControllerSet(CONTROLLER_DOWN);
            		SetInterrupted(true);
            	} else if (strcmp(buf, "NoGame") == 0){
                    ControllerGameSet(GAME_NONE);
            		SetInterrupted(true);
            	} else if (strcmp(buf, "Tetris") == 0){
                    ControllerGameSet(GAME_TETRIS);
            		SetInterrupted(true);
            	} else if (strcmp(buf, "Breakout") == 0){
                    ControllerGameSet(GAME_BREAKOUT);
            		SetInterrupted(true);
            	} else if (strcmp(buf, "Pong") == 0){
                    ControllerGameSet(GAME_PONG);
            		SetInterrupted(true);
            	}
            }
        }
    }
    wificfg_write_string(s, http_redirect_header_controller);
    // close(s);
}

static const char *http_debug_content[] = {
#include "content/wificfg/debug.html"
};

static void handle_debug(int s, wificfg_method method,
                                uint32_t content_length,
                                wificfg_content_type content_type,
                                char *buf, size_t len)
{
    int idx = 0;
    uint16_t ldr_value;
    char tempStr[100];
	printf("%s %d\n", __FUNCTION__, __LINE__);
    if (wificfg_write_string(s, http_success_header) < 0) return;
	printf("%s %d\n", __FUNCTION__, __LINE__);

    if (method != HTTP_METHOD_HEAD) {
    	if (wificfg_write_string(s, http_debug_content[idx++]) < 0) return;
        LdrGetValue16(&ldr_value);
        printf("LDR:%u\n", ldr_value);
        snprintf(tempStr, sizeof(tempStr), "<dt>LDR :</dt> <dd>%d</dd>", ldr_value);
        wificfg_write_string(s, tempStr);
        
        printf("bright:%u\n", ldr_value);
        snprintf(tempStr, sizeof(tempStr), "<dt>Brightness :</dt> <dd>%d</dd>", g_brightness);
        wificfg_write_string(s, tempStr);

        printf("Radio frequency:%dMHz\n", si4703_channel);
        snprintf(tempStr, sizeof(tempStr), "<dt>Radio frequency:</dt> <dd>%.1f MHz</dd>", si4703_channel/10.0);
        wificfg_write_string(s, tempStr);

        printf("RDS text:%s\n", si4703_rds_text);
        snprintf(tempStr, sizeof(tempStr), "<dt>RDS text :</dt> <dd>%s</dd>", si4703_rds_text);
        wificfg_write_string(s, tempStr);

        if (wificfg_write_string(s, http_debug_content[idx++]) < 0) return;

    }
}


/* Minimal not-found response. */
static const char not_found_header[] = "HTTP/1.0 404 \r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "Cache-Control: no-store\r\n"
    "\r\n";


#if configUSE_TRACE_FACILITY
static const char *http_tasks_content[] = {
#include "content/tasks.html"
};

static void handle_tasks(int s, wificfg_method method,
                         uint32_t content_length,
                         wificfg_content_type content_type,
                         char *buf, size_t len)
{
    if (wificfg_write_string(s, http_success_header) < 0) return;

    if (method != HTTP_METHOD_HEAD) {
        if (wificfg_write_string(s, http_tasks_content[0]) < 0) return;
        int num_tasks = uxTaskGetNumberOfTasks();
        xTaskStatusType *task_status = pvPortMalloc(num_tasks * sizeof(xTaskStatusType));

        if (task_status != NULL) {
            int i;

            if (wificfg_write_string(s, "<table><tr><th>Task name</th><th>Task number</th><th>Status</th><th>Priority</th><th>Base priority</th><th>Runtime</th><th>Stack high-water</th></tr>") < 0) return;

            /* Generate the (binary) data. */
            num_tasks = uxTaskGetSystemState(task_status, num_tasks, NULL);

            /* Create a human readable table from the binary data. */
            for(i = 0; i < num_tasks; i++) {
                char cStatus;
                switch(task_status[i].eCurrentState) {
                case eReady: cStatus = 'R'; break;
                case eBlocked: cStatus = 'B'; break;
                case eSuspended: cStatus = 'S'; break;
                case eDeleted: cStatus = 'D'; break;
                default: cStatus = 0x00; break;
                }

                snprintf(buf, len, "<tr><th>%s</th>", task_status[i].pcTaskName);
                if (wificfg_write_string(s, buf) < 0) return;
                snprintf(buf, len, "<td>%u</td><td>%c</td>",
                         (unsigned int)task_status[i].xTaskNumber,
                         cStatus);
                if (wificfg_write_string(s, buf) < 0) return;
                snprintf(buf, len, "<td>%u</td><td>%u</td>",
                         (unsigned int)task_status[i].uxCurrentPriority,
                         (unsigned int)task_status[i].uxBasePriority);
                if (wificfg_write_string(s, buf) < 0) return;
                snprintf(buf, len, "<td>%u</td><td>%u</td></tr>",
                         (unsigned int)task_status[i].ulRunTimeCounter,
                         (unsigned int)task_status[i].usStackHighWaterMark);
                if (wificfg_write_string(s, buf) < 0) return;
            }

            free(task_status);

            if (wificfg_write_string(s, "</table>") < 0) return;
        }
    }

    if (wificfg_write_string(s, http_tasks_content[1]) < 0) return;
}
#endif /* configUSE_TRACE_FACILITY */

static const wificfg_dispatch wificfg_dispatch_list[] = {
    {"/favicon.ico", HTTP_METHOD_GET, handle_favicon, false},
    {"/style.css", HTTP_METHOD_GET, handle_style, false},
    {"/iro.js", HTTP_METHOD_GET, handle_iro, false},
    {"/script.js", HTTP_METHOD_GET, handle_script, false},
    {"/colorpicker.js", HTTP_METHOD_GET, handle_colorPicker, false},
    {"/", HTTP_METHOD_GET, handle_wificfg_redirect, false},
    {"/index.html", HTTP_METHOD_GET, handle_wificfg_redirect, false},
    {"/wificfg/sta.html", HTTP_METHOD_GET, handle_wifi_station, true},
    {"/wificfg/sta.html", HTTP_METHOD_POST, handle_wifi_station_post, true},
    {"/wificfg/clockcfg.html", HTTP_METHOD_GET, handle_clock_cfg, true},
    {"/wificfg/clockcfg.html", HTTP_METHOD_POST, handle_clock_cfg_post, true},
    {"/wificfg/hwcfg.html", HTTP_METHOD_GET, handle_hw_cfg, true},
    {"/wificfg/hwcfg.html", HTTP_METHOD_POST, handle_hw_cfg_post, true},
    {"/wificfg/controller.html", HTTP_METHOD_GET, handle_controller, true},
    {"/wificfg/controller.html", HTTP_METHOD_POST, handle_controller_post, true},
    {"/wificfg/debug.html", HTTP_METHOD_GET, handle_debug, true},
    // {"/wificfg/debug.html", HTTP_METHOD_POST, handle_debug_post, true},
#if configUSE_TRACE_FACILITY
    {"/tasks", HTTP_METHOD_GET, handle_tasks, false},
    {"/tasks.html", HTTP_METHOD_GET, handle_tasks, false},
#endif /* configUSE_TRACE_FACILITY */
    {NULL, HTTP_METHOD_ANY, NULL}
};

typedef struct {
    int32_t port;
    /*
     * Two dispatch lists. First is used for the config pages. Second
     * can be used to extend the pages handled in app code.
     */
    const wificfg_dispatch *wificfg_dispatch;
    const wificfg_dispatch *dispatch;
} server_params;

static void http_server_task(void *pvParameters)
{
    server_params paramss = {80, wificfg_dispatch_list, NULL};
    server_params *params = &paramss;

    struct sockaddr_in serv_addr;
    int listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(params->port);
    
    int enable = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        printf("Error in setsockopt");
        lwip_close(listenfd);
        return;
    }
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 2);

    printf("wificfg server task\n");
    
    bool running = true;
    while (running) {
        uint32_t task_value = 0;
        if (xTaskNotifyWait(0, 1, &task_value, 0) == pdTRUE) {
            if (task_value) {
                running = false;
                break;
            }
        }

        int s = accept(listenfd, (struct sockaddr *)NULL, (socklen_t *)NULL);
        printf("wificfg accept connection\n");
        if (s >= 0) {
            const struct timeval timeout = { 5, 0 };
            setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

            /* Buffer for reading the request and headers and the post method
             * names and values. */
            char buf[48];

            /* Read the request line */
            int request_line_size = read_crlf_line(s, buf, sizeof(buf));
            if (request_line_size < 5) {
                closesocket(s);
                continue;
            }

            /* Parse the http method, path, and protocol version. */
            char *method_end = skip_to_whitespace(buf);
            char *path_string = skip_whitespace(method_end);
            *method_end = 0;
            wificfg_method method = intern_http_method(buf);
            char *path_end = skip_to_whitespace(path_string);
            *path_end = 0;

            /* Dispatch to separate functions to handle the requests. */
            const wificfg_dispatch *match = NULL;
            const wificfg_dispatch *dispatch;

            /*
             * Check the optional application supplied dispatch table
             * first to allow overriding the wifi config paths.
             */
            if (params->dispatch) {
                for (dispatch = params->dispatch; dispatch->path != NULL; dispatch++) {
                    if (strcmp(path_string, dispatch->path) == 0 &&
                        (dispatch->method == HTTP_METHOD_ANY ||
                         method == dispatch->method)) {
                        match = dispatch;
                        break;
                    }
                }
            }

            if (!match) {
                for (dispatch = params->wificfg_dispatch; dispatch->path != NULL; dispatch++) {
                    if (strcmp(path_string, dispatch->path) == 0 &&
                        (dispatch->method == HTTP_METHOD_ANY ||
                         method == dispatch->method)) {
                        match = dispatch;
                        break;
                    }
                }
            }


            /* Read the headers, noting some of interest. */
            long content_length = 0;
            wificfg_content_type content_type = HTTP_CONTENT_TYPE_OTHER;
            bool hostp = false;
            uint32_t host = IPADDR_NONE;

            for (;;) {
                int header_length = read_crlf_line(s, buf, sizeof(buf));
                if (header_length <= 0)
                    break;

                char *name_end = buf;
                for (; ; name_end++) {
                    char c = *name_end;
                    if (!c || c == ':')
                        break;
                }
                if (*name_end == ':') {
                    char *value = name_end + 1;
                    *name_end = 0;
                    http_header header = intern_http_header(buf);
                    value = skip_whitespace(value);
                    switch (header) {
                    case HTTP_HEADER_HOST:
                        hostp = true;
                        host = ipaddr_addr(value);
                        break;
                    case HTTP_HEADER_CONTENT_LENGTH:
                        content_length = strtol(value, NULL, 10);
                        break;
                    case HTTP_HEADER_CONTENT_TYPE:
                        content_type = intern_http_content_type(value);
                        break;
                    default:
                        break;
                    }
                }
            }

            if (hostp && host == IPADDR_NONE) {
                /* Redirect to an IP address. */
                wificfg_write_string(s, http_redirect_header_softap);
            } else if (match) {
                (*match->handler)(s, method, content_length, content_type, buf, sizeof(buf));
            } else {
                wificfg_write_string(s, http_redirect_header_sta);
                printf("Unhandeld request!!: %s\n", path_string);
            }
            lwip_close(s);
        }
    }
    
    printf("Stopping HTTP server");
    lwip_close(listenfd);
    vTaskDelete(NULL);
}

static void wifi_scan_ap_task(void *pvParameters) {
    printf("Starting WiFi scan\n");
    for(int i = 0; i < 30; i++) { //Stop scanning after 5 minutes
        uint32_t task_value = 0;
        if (xTaskNotifyWait(0, 1, &task_value, 0) == pdTRUE) {
            if (task_value) {
                break;
            }
        }
        sdk_wifi_station_scan(NULL, scan_done_cb);
        SleepNI(10 * 1000);
    }
    _wifi_scan_ap_task_handle = NULL;
    vTaskDelete(NULL);
    vSemaphoreDelete(wifi_networks_mutex);
    printf("WiFi scan Stopped\n");
}

void wifi_scan_ap_start() {
    if (_wifi_scan_ap_task_handle) {
        printf("Already scanning access points\n");
        return;
    }
    wifi_networks_mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(wifi_networks_mutex);
    xTaskCreate(wifi_scan_ap_task, "WiFi scan AP", 256, NULL, 3, &_wifi_scan_ap_task_handle);
}

void wifi_scan_ap_stop() {
    if (!_wifi_scan_ap_task_handle) {
        return;
    }
    xTaskNotify(_wifi_scan_ap_task_handle, 1, eSetValueWithOverwrite);
}

void http_server_start() {
    if (_http_server_task_handle) {
        printf("HTTP server already started ERROR!!!!!!!\n");
        return;
    }
    xTaskCreate(http_server_task, "http server", 1024, NULL, 2, &_http_server_task_handle);
}

void http_server_stop() {
    if (!_http_server_task_handle) {
        return;
    }
    xTaskNotify(_http_server_task_handle, 1, eSetValueWithOverwrite);
}
