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

/*
 * The web server parses the http method string in these enums. The ANY method
 * is only use for dispatch. The method enum is passed to the handler functions.
 */
typedef enum {
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_OTHER,
    HTTP_METHOD_ANY,
} wificfg_method;

/*
 * The web server parses these content-type header values. This is passed to the
 * dispatch function.
 */
typedef enum {
    HTTP_CONTENT_TYPE_WWW_FORM_URLENCODED,
    HTTP_CONTENT_TYPE_OTHER
} wificfg_content_type;

/*
 * The function signature for the http server request handler functions.
 *
 * The buffer, with its length, is usable by the handler.
 */
typedef void (* wificfg_handler)(int s, wificfg_method method,
                                 uint32_t content_length,
                                 wificfg_content_type content_type,
                                 char *buf, size_t len);

typedef struct {
    const char *path;
    wificfg_method method;
    wificfg_handler handler;
    bool secure;
} wificfg_dispatch;

//This variable are updated eatch build by the makefile, and version.c
extern const char version[];
extern const char buildDate[];

void wifi_scan_ap_start(void);
void http_server_start();
void http_server_stop();