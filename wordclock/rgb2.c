// Source code from:
// https://www.dmurph.com/posts/2021/1/cabinet-light-3.html

#include "rgb2.h"
#include <stdint.h>
#include <inttypes.h>

// color temperature definition
// https://andi-siess.de/rgb-to-color-temperature/
double WHITE_6000K[] =         {255.0, 243.0, 239.0};
double NATURE_WHITE_4000K[] =  {255.0, 209.0, 163.0};
double WARM_WHITE_3000K[] =    {255.0, 180.0, 107.0};

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

void rgb2rgbw(TPixel dstPixel, TPixel srcPixel, EPixelType ledstripType) {
  double *whiteLedRGBDef;

  switch (ledstripType) {
    case PIXEL_TYPE_RGBW:
      whiteLedRGBDef = WHITE_6000K;
    break;
    case PIXEL_TYPE_RGBNW:
      whiteLedRGBDef = NATURE_WHITE_4000K;
    break;
    case PIXEL_TYPE_RGBWW:
      whiteLedRGBDef = WARM_WHITE_3000K;
    break;
    case PIXEL_TYPE_RGB:
    case PIXEL_TYPE_RGB_N_W:
    default:
      dstPixel[RED_IDX] = srcPixel[RED_IDX];
      dstPixel[GREEN_IDX] = srcPixel[GREEN_IDX];
      dstPixel[BLUE_IDX] = srcPixel[BLUE_IDX];
      dstPixel[WHITE_IDX] = 0; //don't use the white led
      return;
    break;
  }

  // These values are what the 'white' value would need to
  // be to get the corresponding color value.
  double whiteValueForRed   = ((double)srcPixel[RED_IDX]   * 255.0) / whiteLedRGBDef[0];
  double whiteValueForGreen = ((double)srcPixel[GREEN_IDX] * 255.0) / whiteLedRGBDef[1];
  double whiteValueForBlue  = ((double)srcPixel[BLUE_IDX]  * 255.0) / whiteLedRGBDef[2];

  // Set the white value to the highest it can be for the given color
  // (without over saturating any channel - thus the minimum of them).
  double minWhiteValue = MIN(whiteValueForRed, MIN(whiteValueForGreen, whiteValueForBlue));

  // The rest of the channels will just be the original value minus the
  // contribution by the white channel.
  dstPixel[RED_IDX]   = (uint8_t)((double)srcPixel[RED_IDX]   - minWhiteValue * whiteLedRGBDef[0] / 255.0);
  dstPixel[GREEN_IDX] = (uint8_t)((double)srcPixel[GREEN_IDX] - minWhiteValue * whiteLedRGBDef[1] / 255.0);
  dstPixel[BLUE_IDX]  = (uint8_t)((double)srcPixel[BLUE_IDX]  - minWhiteValue * whiteLedRGBDef[2] / 255.0); 
  dstPixel[WHITE_IDX] = (minWhiteValue <= 255.0 ? (uint8_t) minWhiteValue : 255);
}