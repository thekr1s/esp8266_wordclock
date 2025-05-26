/*
 * ldr.c
 *
 *  Created on: Oct 26, 2016
 *      Author: robert
 */

#include <stdio.h>
#include "ldr.h"
#include <espressif/esp_system.h>

void LdrInit(void){
}

uint16_t LdrGetValue16(void) {
	return sdk_system_adc_read();
}
