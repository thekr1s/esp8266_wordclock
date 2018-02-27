/*
 * ldr.c
 *
 *  Created on: Oct 26, 2016
 *      Author: robert
 */

#include <stdio.h>
#include "ldr.h"
#include <espressif/esp_system.h>

void LdrInit(){

}

void LdrGetValue16(uint16_t* v) {
	*v = sdk_system_adc_read();
	//printf("%s %d\n", __FUNCTION__, *v);
}
