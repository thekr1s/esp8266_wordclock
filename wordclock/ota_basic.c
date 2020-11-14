/* A very simple OTA example
 *
 * Tries to run both a TFTP client and a TFTP server simultaneously, either will accept a TTP firmware and update it.
 *
 * Not a realistic OTA setup, this needs adapting (choose either client or server) before you'd want to use it.
 *
 * For more information about esp-open-rtos OTA see https://github.com/SuperHouse/esp-open-rtos/wiki/OTA-Update-Configuration
 *
 * NOT SUITABLE TO PUT ON THE INTERNET OR INTO A PRODUCTION ENVIRONMENT!!!!
 */
#include <string.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"
#include "mbedtls/sha256.h"

#include "ota-tftp.h"
#include "rboot-api.h"
#include "esp_glue.h"
#include "settings.h"

/* TFTP client will request this image filenames from this server */
#define TFTP_IMAGE_FILENAME_RELEASE "woordklok.bin"
#define TFTP_IMAGE_FILENAME_DEBUG   "woordklok_dbg.bin"

static bool _isBusy = false;

static const char* get_fw_filename(bool prepend_slash) {
    if (g_settings.otaFwType == OTA_FW_RELEASE) {
        if (prepend_slash) {
            return "/" TFTP_IMAGE_FILENAME_RELEASE;

        } else {
            return TFTP_IMAGE_FILENAME_RELEASE;
        }
    } else {
        if (prepend_slash) {
            return "/" TFTP_IMAGE_FILENAME_DEBUG;
        } else {
            return TFTP_IMAGE_FILENAME_DEBUG;
        }
    }
}

/* Example function to TFTP download a firmware file and verify its SHA256 before
   booting into it.
*/
static void tftpclient_download_and_verify_file1(int slot, rboot_config *conf)
{
    int res;

    printf("Downloading %s to slot %d...\n", get_fw_filename(false), slot);
    res = ota_tftp_download(g_settings.otaFwUrl, atoi(g_settings.otaFwPort), get_fw_filename(false), 1000, slot, NULL);
    printf("ota_tftp_download %s result %d\n", get_fw_filename(false), res);

    if (res != 0) {
        return;
    }

    printf("Looks valid, calculating SHA256...\n");
    uint32_t length;
    bool valid = rboot_verify_image(conf->roms[slot], &length, NULL);
    static mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    valid = valid && rboot_digest_image(conf->roms[slot], length, (rboot_digest_update_fn)mbedtls_sha256_update, &ctx);
    static uint8_t hash_result[32];
    mbedtls_sha256_finish(&ctx, hash_result);
    mbedtls_sha256_free(&ctx);

    if(!valid)
    {
        printf("Not valid after all :(\n");
        return;
    }
    rboot_set_current_rom(slot);
    Sleep(1000);
    sdk_system_restart();
}

static void tftp_client_task(void *pvParameters)
{
	int retries = 3;
    printf("TFTP client task starting...\n");
    rboot_config conf;
    conf = rboot_get_config();
    int slot = (conf.current_rom + 1) % conf.count;
    printf("Image will be saved in OTA slot %d.\n", slot);
    if(slot == conf.current_rom) {
        printf("FATAL ERROR: Only one OTA slot is configured!\n");
        return;
    }

    _isBusy = true;
    /* Alternate between trying two different filenames. Probalby want to change this if making a practical
       example!

       Note: example will reboot into FILENAME1 if it is successfully downloaded, but FILENAME2 is ignored.
    */
    while(retries > 0) {
        tftpclient_download_and_verify_file1(slot, &conf);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        retries--;

    }
    _isBusy = false;
    vTaskDelete( NULL );
}

bool OtaIsBusy(void){
	return _isBusy;
}

#include "http_client_ota.h"

static inline void ota_error_handling(OTA_err err) {
    printf("OTA update done\r\nstatus: ");

    switch(err) {
    case OTA_DNS_LOOKUP_FALLIED:
        printf("DNS lookup has fallied\n");
        break;
    case OTA_SOCKET_ALLOCATION_FALLIED:
        printf("Impossible allocate required socket\n");
        break;
    case OTA_SOCKET_CONNECTION_FALLIED:
        printf("Server unreachable, impossible connect\n");
        break;
    case OTA_SHA_DONT_MATCH:
        printf("Sha256 sum does not fit downloaded sha256\n");
        break;
    case OTA_REQUEST_SEND_FALLIED:
        printf("Impossible send HTTP request\n");
        break;
    case OTA_DOWLOAD_SIZE_NOT_MATCH:
        printf("Dowload size don't match with server declared size\n");
        break;
    case OTA_ONE_SLOT_ONLY:
        printf("rboot has only one slot configured, impossible switch it\n");
        break;
    case OTA_FAIL_SET_NEW_SLOT:
        printf("rboot cannot switch between rom\n");
        break;
    case OTA_IMAGE_VERIFY_FALLIED:
        printf("Dowloaded image binary checsum is fallied\n");
        break;
    case OTA_UPDATE_DONE:
        printf("Ota has completed upgrade process, all ready for system software reset\n");
        break;
    case OTA_HTTP_OK:
        printf("HTTP server has response 200, Ok\n");
        break;
    case OTA_HTTP_NOTFOUND:
        printf("HTTP server has response 404, file not found\n");
        break;
    }
}

static void ota_http_task(void *PvParameter)
{
    ota_info* pInfo = (ota_info *)PvParameter;
    _isBusy = true;
    int tryCount = 2;

    while (tryCount > 0) {
        OTA_err err;
        tryCount--;
        // Remake this task until ota work
        printf("HTTP OTA start download %s:%s%s\r\n", pInfo->server, pInfo->port, pInfo->binary_path);
        err = ota_update(pInfo);

        ota_error_handling(err);

        if(err != OTA_UPDATE_DONE) {
            Sleep(1000);
            printf("\n\n\n");
            continue;
        } 

        Sleep(1000);
        printf("Reset\n");
        sdk_system_restart();
    }

    _isBusy = false;
     vTaskDelete( NULL );


}
static ota_info info = {
    .server      = "",
    .port        = "",
    .binary_path = TFTP_IMAGE_FILENAME_DEBUG,
    .sha256_path = NULL,
};

void OtaUpdateInit(void)
{
	if (_isBusy) return;

    rboot_config conf = rboot_get_config();
    printf("\r\n\r\nCurrently running on flash slot %d / %d.\r\n\r\n",
           conf.current_rom, conf.count);

    printf("Image addresses in flash:\r\n");
    for(int i = 0; i <conf.count; i++) {
        printf("%c%d: offset 0x%08x\r\n", i == conf.current_rom ? '*':' ', i, conf.roms[i]);
    }

    if (strncmp("http://", g_settings.otaFwUrl, 7) == 0) {
        // retrieve via http
        info.server = &g_settings.otaFwUrl[7]; // strip the http://
        info.port = g_settings.otaFwPort;
        info.binary_path = get_fw_filename(true);
        xTaskCreate(&ota_http_task, "http_ota_client", 4096, &info, 2, NULL);
    } else {
        // Use TFTP
        xTaskCreate(&tftp_client_task, "tftp_ota_client", 2048, NULL, 2, NULL);
    }
}
