/*
 * rgb2.h
 *
 *  Created on: Oct 4, 2021
 *      Author: rutger.huijgen
 */

#ifndef RGB2_H_
#define RGB2_H_

#include <stdint.h>
#include "settings.h"
#include "AddressableLedStrip.h"

void rgb2rgbw(TPixel dstPixel, TPixel srcPixel, EPixelType ledstripType);

#endif