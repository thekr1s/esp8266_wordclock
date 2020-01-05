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
#include "ssid_config.h"
#include "mbedtls/sha256.h"

#include "ota-tftp.h"
#include "rboot-api.h"
#include "esp_glue.h"
#include "settings.h"

/* TFTP client will request this image filenames from this server */
#define TFTP_IMAGE_FILENAME1 "ws2812_buffer.bin"

static bool _isBusy = false;

/* Example function to TFTP download a firmware file and verify its SHA256 before
   booting into it.
*/
static void tftpclient_download_and_verify_file1(int slot, rboot_config *conf)
{
    printf("Downloading %s to slot %d...\n", TFTP_IMAGE_FILENAME1, slot);
    int res;
    

    res = ota_tftp_download(g_settings.otaFwUrl, atoi(g_settings.otaFwPort), TFTP_IMAGE_FILENAME1, 1000, slot, NULL);
    printf("ota_tftp_download %s result %d\n", TFTP_IMAGE_FILENAME1, res);

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

    xTaskCreate(&tftp_client_task, "tftp_client", 2048, NULL, 2, NULL);
}
