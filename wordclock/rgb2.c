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

void rgb2rgbw(TPixel pixel, EPixelType ledstripType) {
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
    case PIXEL_TYPE_RGB_N_W:
      pixel[WHITE_IDX] = 0; //don't use  the white led
    case PIXEL_TYPE_RGB:
    default:
      return; //This ledstrip doesn't need converstion
    break;
  }

  //Get the maximum between R, G, and B
  uint8_t r = pixel[RED_IDX];
  uint8_t g = pixel[GREEN_IDX];
  uint8_t b = pixel[BLUE_IDX];
  //uint8_t w = pixel[BLUE_IDX];

  // These values are what the 'white' value would need to
  // be to get the corresponding color value.
  double whiteValueForRed = ((double)r * 255.0) / whiteLedRGBDef[0];
  double whiteValueForGreen = ((double)g * 255.0) / whiteLedRGBDef[1];
  double whiteValueForBlue = ((double)b * 255.0) / whiteLedRGBDef[2];

  // Set the white value to the highest it can be for the given color
  // (without over saturating any channel - thus the minimum of them).
  double minWhiteValue = MIN(whiteValueForRed, MIN(whiteValueForGreen, whiteValueForBlue));

  // The rest of the channels will just be the original value minus the
  // contribution by the white channel.
  pixel[RED_IDX] = (uint8_t)((double)r - minWhiteValue * whiteLedRGBDef[0] / 255.0);
  pixel[GREEN_IDX] = (uint8_t)((double)g - minWhiteValue * whiteLedRGBDef[1] / 255.0);
  pixel[BLUE_IDX] = (uint8_t)((double)b - minWhiteValue * whiteLedRGBDef[2] / 255.0); 
  pixel[WHITE_IDX] = (minWhiteValue <= 255 ? (uint8_t) minWhiteValue : 255);
}